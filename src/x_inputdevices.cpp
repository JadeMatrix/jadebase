/* 
 * x_inputdevices.cpp
 * 
 * About
 * 
 */

#ifndef PLATFORM_XWS_GNUPOSIX
#error PLATFORM_XWS_GNUPOSIX not defined
#endif

/* INCLUDES *******************************************************************//******************************************************************************/

#include "x_inputdevices.hpp"

#include <map>
#include <set>
#include <cmath>

#include "bqt_log.hpp"
#include "bqt_exception.hpp"
#include "bqt_launchargs.hpp"
#include "bqt_preferences.hpp"
#include "bqt_windowevent.hpp"
#include "bqt_windowmanagement.hpp"
#include "threading/bqt_mutex.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex idev_mutex;                                                      // Isn't needed now since events are single-threaded, but may eventually
    
    // INPUT DEVICE INFO ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::set< Atom > supported_device_types;
    const char* atom_strings[] = { XI_MOUSE,
                                   XI_TABLET,
                                   XI_TOUCHSCREEN,
                                   "STYLUS",
                                   "ERASER",
                                   "TOUCH",
                                   // "AIRBRUSH",
                                   "TOUCHPAD" };                                // For initializing supported_device_types
    #define SUPPORTED_DEVICE_TYPE_COUNT ( sizeof( atom_strings ) / sizeof( const char* ) )
    
    void initSupportedDeviceTypes()                                             // Safe to call multiple times
    {
        Display* x_display = getXDisplay();
        
        for( int i = 0;
             i < ( SUPPORTED_DEVICE_TYPE_COUNT );
             ++i )
        {
            Atom a = XInternAtom( x_display, atom_strings[ i ], true );         // Get atom only if it exists
            
            if( a )                                                             // Only insert if atom exists
                supported_device_types.insert( a );
        }
    }
    
    typedef int event_type;
    
    enum pen_type
    {
        INVALID,
        PEN_STYLUS,
        AIRBRUSH_STYLUS,
        ERASER_STYLUS,
        TOUCH_STYLUS
    };
    
    struct x_input_detail
    {
        std::string name;                                                       // std::string instead of const char* so we copy the string easily
        
        XID x_devid;                                                            // aka bqt_platform_idevid_t under X
        XDevice* x_device;
        
        bool relative;
        int axis_count;
        XAxisInfo axes[ 6 ];
        pen_type type;
    };
    
    std::map< bqt_platform_idevid_t, x_input_detail > x_input_devices;
    
    struct event_detail
    {
        enum
        {
            BUTTON_PRESS,
            BUTTON_RELEASE,
            MOTION
        } event_class;
        bqt_platform_idevid_t x_devid;
    };
    
    std::map< event_type, event_detail > event_type_map;
    
    int x_total_device_count = 0;                                               // Needed for checking if there are new devices
    
    // INPUT EVENT ACCUMULATING ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::map< bqt_platform_idevid_t,
              bqt::stroke_waypoint > prev_motion_events;
    
    static bqt_platform_idevid_t dummy_idevid = 0x00;                           // Could have used 0x00, but xinput lists Virtual Core Pointer as 2
    static bqt::stroke_waypoint initial_waypoint = { dummy_idevid,
                                                     0x00,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     { -INFINITY, -INFINITY },  // Element masking checks left-right, so we want to fail early
                                                     { -INFINITY, -INFINITY },
                                                     0.0f,
                                                     { 0.0f, 0.0f },
                                                     0.0f,
                                                     0.0f };
}

/******************************************************************************//******************************************************************************/

#define DEVICE_CLASS_LIST_START_LENGTH  32

namespace bqt
{
    void openInputDevices()
    {
        scoped_lock< mutex > slock( idev_mutex );
        
        Display* x_display = getXDisplay();
        int x_screen = DefaultScreen( x_display );
        
        std::map< bqt_platform_idevid_t, x_input_detail > new_devices;          // For swapping with old list
        
        if( getDevMode() )
            ff::write( bqt_out, "Opening input devices\n" );
        
        int x_eventclass_size = DEVICE_CLASS_LIST_START_LENGTH;                 // Max devices is 128 so int is OK
        XEventClass* x_eventclass_list = new XEventClass[ x_eventclass_size ];
        int x_eventclass_count = 0;
        
        float x_screen_res[ 2 ];                                                // This is used for checking the resolution of device axes
        {
            Screen* x_screen_ptr = DefaultScreenOfDisplay( x_display );
            x_screen_res[ 0 ] = WidthMMOfScreen( x_screen_ptr ) / WidthOfScreen( x_screen_ptr );
            x_screen_res[ 1 ] = HeightMMOfScreen( x_screen_ptr ) / HeightOfScreen( x_screen_ptr );
        }
        
        XDeviceInfo* x_dev_info = XListInputDevices( x_display,
                                                     &x_total_device_count );   // Get device list & update global count
        {
            if( x_dev_info == NULL )
                throw exception( "openInputDevices(): Failed to get list of X devices" );
            
            if( supported_device_types.empty() )
                initSupportedDeviceTypes();
            
            if( supported_device_types.size() == SUPPORTED_DEVICE_TYPE_COUNT )
            {
                if( getDevMode() )
                    ff::write( bqt_out, "All supported device types available\n" );
            }
            else
            {
                if( getDevMode() )
                    ff::write( bqt_out, "Not all supported device types available, some functionality might be missing\n" );
            }
            
            for( int i = 0; i < x_total_device_count; i++ )
            {
                if( getDevMode() )
                    ff::write( bqt_out,
                               "Found a device: \"",
                               x_dev_info[ i ].name,
                               "\" as id ",
                               x_dev_info[ i ].id,
                               "\n" );
                
                if( x_dev_info[ i ].use == IsXKeyboard
                    || x_dev_info[ i ].use == IsXPointer )                      // Ignore master devices, they are most likely virtual
                {
                    if( getDevMode() )
                        ff::write( bqt_out, "  - Master device, skipping\n" );
                    continue;
                }
                
                if( x_dev_info[ i ].type == 0x00 )                              // Supported device type is unavailable
                {
                    if( getDevMode() )
                        ff::write( bqt_out, "  - Unavailable device type, skipping\n" );
                    continue;
                }
                
                {
                    for( std::map< bqt_platform_idevid_t, x_input_detail >::iterator iter = x_input_devices.begin();
                        iter != x_input_devices.end();
                        ++iter )                                                // If device is already open, add it to new_devices & erase from x_input_devices
                    {
                        if( iter -> second.x_devid == x_dev_info[ i ].id )
                        {
                            if( getDevMode() )
                                ff::write( bqt_out, "  - Device already open, skipping\n" );
                            
                            new_devices[ iter -> second.x_devid ] = iter -> second;
                            x_input_devices.erase( iter );
                            
                            goto next_device;
                        }
                    }
                    
                    if( supported_device_types.count( x_dev_info[ i ].type ) )  // Device type is supported
                    {
                        x_input_detail detail;
                        
                        bool register_device = false;
                        
                        detail.type     = PEN_STYLUS;
                        detail.name     = x_dev_info[ i ].name;
                        detail.x_devid  = x_dev_info[ i ].id;
                        detail.x_device = XOpenDevice( x_display, detail.x_devid  );
                        
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        
                        // Much thanks to
                        // http://mobileim.googlecode.com/svn/MeeGo/meegoTraning/MeeGo_Traing_Doc_0119/meego_trn/meego-quality-assurance-mcts/mcts-blts/blts-x11/src/xinput_tests.c
                        // for demonstrating how to use XDeviceInfo.
                        
                        XAnyClassPtr x_any = ( XAnyClassPtr )x_dev_info[ i ].inputclassinfo;
                        
                        for( int j = 0; j < x_dev_info[ i ].num_classes; ++j )
                        {
                            if( sizeof( XID ) != sizeof( long ) )               // TODO: Find a way to make this a compile-time error
                                throw exception( "openTabletDevices(): X class 'class' member hack does not work" );
                            // switch( x_any -> class )                         // Can't access XAny::class directly as 'class' is a keyword in C++
                            switch( *( ( long* )x_any  ) )                      // Luckily we know that class should be the first member of XAny
                            {
                            case ButtonClass:
                                {
                                    XButtonInfo* xbutt_info = ( XButtonInfo* )x_any;
                                    
                                    if( getDevMode() )
                                        ff::write( bqt_out,
                                                   "  - ",
                                                   xbutt_info -> num_buttons,
                                                   " buttons\n" );
                                    
                                    event_type x_event;
                                    
                                    DeviceButtonPress( detail.x_device,
                                                       x_event,
                                                       x_eventclass_list[ x_eventclass_count ] );
                                    ++x_eventclass_count;                       // DeviceMotionNotify() is a macro; we have to increment outside
                                    event_type_map[ x_event ].x_devid = detail.x_devid;
                                    event_type_map[ x_event ].event_class = event_detail::BUTTON_PRESS;
                                    
                                    DeviceButtonRelease( detail.x_device,
                                                         x_event,
                                                         x_eventclass_list[ x_eventclass_count ] );
                                    ++x_eventclass_count;
                                    event_type_map[ x_event ].x_devid = detail.x_devid;
                                    event_type_map[ x_event ].event_class = event_detail::BUTTON_RELEASE;
                                    
                                    register_device = true;
                                }
                                break;
                            case ValuatorClass:
                                {
                                    XValuatorInfo* xval_info = ( XValuatorInfo* )x_any;
                                    
                                    switch( xval_info -> mode )
                                    {
                                    case Absolute:
                                        detail.relative = false;
                                        break;
                                    case Relative:
                                        detail.relative = true;
                                        break;
                                    default:
                                        ff::write( bqt_out, "Warning: Input device found in neither absolute nor relative mode, ignoring\n" );
                                        goto next_device;
                                        break;
                                    }
                                    
                                    detail.axis_count = xval_info -> num_axes;
                                    
                                    if( getDevMode() )
                                        ff::write( bqt_out,
                                                   "  - ",
                                                   detail.axis_count,
                                                   " axes\n" );
                                    
                                    for( int axis = 0; i < detail.axis_count; ++i )
                                        detail.axes[ axis ] = xval_info -> axes[ axis ];
                                    
                                    event_type x_event;
                                    DeviceMotionNotify( detail.x_device,
                                                        x_event,
                                                        x_eventclass_list[ x_eventclass_count ] );
                                    ++x_eventclass_count;
                                    
                                    event_type_map[ x_event ].x_devid = detail.x_devid;
                                    event_type_map[ x_event ].event_class = event_detail::MOTION;
                                    
                                    register_device = true;
                                }
                                break;
                            default:
                                break;                                          // Ignore others (eg KeyClass)
                            }
                            
                            if( register_device )
                            {
                                if( 1000 / detail.axes[ 0 ].resolution          // If the device l/mm is larger than the smallest screen l/mm
                                                                                // (We just check the first axis, that should be OK)
                                    > ( x_screen_res[ 0 ] < x_screen_res[ 1 ] ? x_screen_res[ 0 ] : x_screen_res[ 1 ] ) )
                                {
                                    ff::write( bqt_out,
                                               "Warning: Device \"",
                                               detail.name,
                                               "\" has a larger resolution than the screen\n" );
                                }
                                
                                new_devices[ detail.x_devid ] = detail;         // Add the device to the new list
                            }
                            
                            if( x_eventclass_count >= DEVICE_CLASS_LIST_START_LENGTH )
                            {
                                ff::write( bqt_out, "Warning: Max input device events registered, ignoring rest\n" );
                                goto devices_finish;
                                
                                // Tried using memcpy to expand x_eventclass_list, but
                                // XLib (actually XCB) just errored out.  Considered
                                // using std::vector::data(), but that might get a bit
                                // messy.
                            }
                            // {
                            //     XEventClass* old = x_eventclass_list;
                            //     x_eventclass_list = ( XEventClass* )memcpy( new XDeviceInfo[ x_eventclass_size * 2 ],
                            //                                                 x_eventclass_list,
                            //                                                 x_eventclass_size );    // Reallocate double the space * copy
                            //     // x_eventclass_list = ( XEventClass* )memcpy( ( XEventClass* )calloc( x_eventclass_size * 2, sizeof( XEventClass ) ),
                            //     //                                             x_eventclass_list,
                            //     //                                             x_eventclass_size );
                            //     x_eventclass_size *= 2;
                            //     delete[] old;
                            //     // free( old );
                            //     ff::write( bqt_out, "successfully copied x_eventclass_list, expanded to ", x_eventclass_size, "\n" );
                            // }
                            
                            x_any = ( XAnyClassPtr )( ( char* )x_any
                                                      + x_any -> length );      // This unusual method is required as XDeviceInfo::inputclassinfo is an array
                                                                                // with variable-length members
                        }
                        
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    }
                    else
                        if( getDevMode() )
                        {
                            ff::write( bqt_out,
                                       "  - Unsupported device type (atom \"",
                                       XGetAtomName( x_display, x_dev_info[ i ].type ),
                                       "\"), skipping\n" );
                        }
                }
                
            next_device:
                NULL;
            }
        }
    devices_finish:
        XFreeDeviceList( x_dev_info );
        
        if( getDevMode() && new_devices.size() == 0 )
            ff::write( bqt_out, "No usable devices found\n" );
        
        XSelectExtensionEvent( x_display,
                               RootWindow( x_display, x_screen ),
                               x_eventclass_list,
                               x_eventclass_count );                            // Tell X we want these events
        
        x_input_devices.swap( new_devices );
        
        for( std::map< bqt_platform_idevid_t, x_input_detail >::iterator newdev_iter = new_devices.begin();
             newdev_iter != new_devices.end();
             ++newdev_iter )                                                    // Clean up previous motion events & close unplugged devices (if possible)
                                                                                // At this point new_devices will only hold devics no longer available
        {
            prev_motion_events.erase( newdev_iter -> second.x_devid );
            
            for( std::map< event_type, event_detail >::iterator event_iter = event_type_map.begin();
                 event_iter != event_type_map.end();
                 ++event_iter )                                                 // Remove all associated XInput event types
            {
                if( event_iter -> second.x_devid == newdev_iter -> second.x_devid )
                {
                    event_type_map.erase( event_iter );
                    // Continue since there might be more
                }
            }
            
            if( getDevMode() )
                ff::write( bqt_out,
                           "Leaking device \"",
                           newdev_iter -> second.name,
                           "\" (unplugged?)\n" );                               // If a device was unplugged while open, trying to close it will result in an
                                                                                // error.  The only option is to leak the memory.  This project concurs:
                                                                                // https://github.com/Alcaro/minir/blob/master/inputraw-x11-xinput2.c
            // XCloseDevice( x_display, iter -> x_device );                     // Ideally we'd be able to do this instead
        }
    }
    void closeInputDevices()
    {
        scoped_lock< mutex > slock( idev_mutex );
        
        if( getDevMode() )
            ff::write( bqt_out, "Closing input devices\n" );
        
        Display* x_display = getXDisplay();
        
        for( std::map< bqt_platform_idevid_t, x_input_detail >::iterator iter = x_input_devices.begin();
             iter != x_input_devices.end();
             ++iter )
        {
            // TODO: Check for errors when closing (null ptr, etc.) as devices just might be unplugged during this function
            XCloseDevice( x_display, iter -> second.x_device );
            
            if( getDevMode() )
                ff::write( bqt_out, "Closed device \"", iter -> second.name, "\"\n" );
        }
        
        prev_motion_events.clear();
        event_type_map.clear();
    }
    void refreshInputDevices()
    {
        scoped_lock< mutex > slock( idev_mutex );
        
        Display* x_display = getXDisplay();
        
        int new_device_count;
        
        #if USE_DMX                                                             // This has not been tested or even compiled; it is merely an example of what
                                                                                // could be done if we were using DMX
        
        int dmx_event_basep;                                                    // Unused
        int dmx_error_basep;                                                    // Unused
        static bool dmx_unavailable_flag = false;
        
        if( DMXQueryExtension( x_display, &dmx_event_basep, &dmx_error_basep ) )
        {
            if( !DMXGetInputCount( x_display, &new_device_count ) )
                throw exception( "HandleEvents_task::execute(): Could not get input device count" );
        }
        else
        {
            if( !dmx_unavailable_flag && getDevMode() )
            {
                ff::write( bqt_out, "DMX unavailable\n" );
                dmx_unavailable_flag = true;
            }
            
            XFreeDeviceList( XListInputDevices( x_display,
                                                &new_device_count ) );
        }
        
        #else
        
        XFreeDeviceList( XListInputDevices( x_display,
                                            &new_device_count ) );              // No clear way of getting just the count
        
        #endif
        
        if( new_device_count != x_total_device_count )
        {
            openInputDevices();
        }
    }
    
    void handleStrokeEvent( XEvent& x_event )
    {
        scoped_lock< mutex > slock( idev_mutex );
                
        bqt::window* target = bqt::getActiveWindow();
        if( target == NULL )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Warning: Got motion event not matched to a known window, ignoring\n" );
            
            return;
        }
        
        if( event_type_map.count( x_event.type ) )
        {
            bqt::window_event w_event;
            w_event.type = NONE;
            
            x_input_detail& device_detail( x_input_devices[ event_type_map[ x_event.type ].x_devid ] );
            
            switch( event_type_map[ x_event.type ].event_class )
            {
            case event_detail::BUTTON_PRESS:
                {
                    
                }
                break;
            case event_detail::BUTTON_RELEASE:
                {
                    
                }
                break;
            case event_detail::MOTION:
                {
                    
                }
                break;
            default:
                throw exception( "handleStrokeEvent(): Unknown input event class" );
            }
            
            if( w_event.type != NONE )
                target -> acceptEvent( w_event );
        }
        else
            if( getDevMode() )
            {
                static std::set< event_type > unknown_event_types;
                
                if( unknown_event_types.count( x_event.type ) == 0 )
                {
                    unknown_event_types.insert( x_event.type );
                    
                    ff::write( bqt_out,
                               "Warning: Got unknown event type (",
                               x_event.type,
                               "), ignoring this type until registered\n" );
                }
            }
    }
}


