/* 
 * x_inputdevices.cpp
 * 
 * http://www.x.org/archive/X11R7.5/doc/man/man3/XOpenDevice.3.html
 * http://linux.die.net/man/3/xlistinputdevices
 * http://tronche.com/gui/x/xlib/events/structures.html#XEvent
 * 
 * Use xev to listen to Xlib events (does not report DeviceMotion)
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
    
    enum event_class
    {
        BUTTON_PRESS,
        BUTTON_RELEASE,
        MOTION
    };
    
    std::map< event_type, event_class > event_type_map;                         // Event types are per-class, not per-device-class, so we only need ~3
    
    int x_total_device_count = 0;                                               // Needed for checking if there are new devices
    
    // INPUT EVENT ACCUMULATING ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::map< bqt_platform_idevid_t,
              bqt::stroke_waypoint > prev_strokes;
    
    static bqt::stroke_waypoint initial_waypoint = { 0x00,
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
        
        // TODO: Re-think how this might work
        // float x_screen_lpmm[ 2 ];                                               // This is used for checking the resolution of device axes
        // {
        //     Screen* x_screen_ptr = DefaultScreenOfDisplay( x_display );
        //     x_screen_lpmm[ 0 ] = ( float )WidthOfScreen( x_screen_ptr ) / ( float )WidthMMOfScreen( x_screen_ptr );
        //     x_screen_lpmm[ 1 ] = ( float )HeightOfScreen( x_screen_ptr ) / ( float )HeightMMOfScreen( x_screen_ptr );
        // }
        
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
                                    event_type_map[ x_event ] = BUTTON_PRESS;
                                    
                                    DeviceButtonRelease( detail.x_device,
                                                         x_event,
                                                         x_eventclass_list[ x_eventclass_count ] );
                                    ++x_eventclass_count;
                                    event_type_map[ x_event ] = BUTTON_RELEASE;
                                    
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
                                    
                                    for( int axis = 0; axis < detail.axis_count; ++axis )
                                        detail.axes[ axis ] = xval_info -> axes[ axis ];
                                    
                                    event_type x_event;
                                    DeviceMotionNotify( detail.x_device,
                                                        x_event,
                                                        x_eventclass_list[ x_eventclass_count ] );
                                    ++x_eventclass_count;
                                    event_type_map[ x_event ] = MOTION;
                                    
                                    register_device = true;
                                }
                                break;
                            default:
                                break;                                          // Ignore others (eg KeyClass)
                            }
                            
                            if( j + 1 == x_dev_info[ i ].num_classes
                                && register_device )                            // If last one
                            {
                                // if( ( float )( detail.axes[ 0 ].resolution ) / 1000.0f // Axes resolution reported in lines per meter
                                //     < ( x_screen_lpmm[ 0 ] < x_screen_lpmm[ 1 ] ? x_screen_lpmm[ 0 ] : x_screen_lpmm[ 1 ] ) )
                                // {
                                //     ff::write( bqt_out,
                                //                "  - Warning: Device \"",
                                //                detail.name,
                                //                "\" has a lower resolution (",
                                //                ( float )( detail.axes[ 0 ].resolution ) / 1000.0f,
                                //                " l/mm) than the screen (",
                                //                ( x_screen_lpmm[ 0 ] < x_screen_lpmm[ 1 ] ? x_screen_lpmm[ 0 ] : x_screen_lpmm[ 1 ] ),
                                //                " l/mm)\n" );
                                // }
                                
                                if( x_dev_info[ i ].type == XInternAtom( x_display, "ERASER", true ) )
                                {
                                    detail.type = ERASER_STYLUS;
                                }
                                else if( x_dev_info[ i ].type == XInternAtom( x_display, XI_TOUCHSCREEN, true )
                                         || x_dev_info[ i ].type == XInternAtom( x_display, "TOUCH", true )
                                         || x_dev_info[ i ].type == XInternAtom( x_display, "TOUCHPAD", true ) )
                                {
                                    detail.type = TOUCH_STYLUS;
                                }
                                else
                                {
                                    detail.type = PEN_STYLUS;
                                }
                                
                                new_devices[ detail.x_devid ] = detail;         // Add the device to the new list
                                
                                prev_strokes[ detail.x_devid ] = initial_waypoint;
                                                                                // Load initial waypoint, no need to change its dev_id
                                
                                ff::write( bqt_out, "  - device registered\n" );
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
            prev_strokes.erase( newdev_iter -> second.x_devid );
            
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
        
        prev_strokes.clear();
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
                
        static bool warn_relative = true;                                       // Relative motion event resolution warning flag
        
        // ff::write( bqt_out, "got stroke event in window 0x", ff::to_x( ( unsigned long )x_event.xany.window ), "\n" );
        
        bqt_platform_window_t target_pwin;
        target_pwin.x_window = ( ( XDeviceMotionEvent* )&x_event ) -> window;
        
        if( !isRegisteredWindow( target_pwin) )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Warning: Got motion event not matched to a known window, ignoring\n" );
            
            return;
        }
        bqt::window& target = getWindow( target_pwin );
        
        event_type x_eventtype;
        
        if( event_type_map.count( x_event.type ) )
            x_eventtype = event_type_map[ x_event.type ];
        else
            if( getDevMode() )                                                  // Ignore unknown event types
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
                
                return;
            }
        
        XDeviceButtonEvent x_eventdata;                                         // Use an XDeviceButtonEvent since it holds all the info we need
        if( x_eventtype == MOTION )
        {
            XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&x_event ) );
            
            x_eventdata.type         = x_dmevent.type;
            x_eventdata.serial       = x_dmevent.serial;
            x_eventdata.send_event   = x_dmevent.send_event;
            x_eventdata.display      = x_dmevent.display;
            x_eventdata.window       = x_dmevent.window;
            x_eventdata.deviceid     = x_dmevent.deviceid;
            x_eventdata.root         = x_dmevent.root;
            x_eventdata.subwindow    = x_dmevent.subwindow;
            x_eventdata.time         = x_dmevent.time;
            x_eventdata.x            = x_dmevent.x;
            x_eventdata.y            = x_dmevent.y;
            x_eventdata.x_root       = x_dmevent.x_root;
            x_eventdata.y_root       = x_dmevent.y_root;
            x_eventdata.state        = x_dmevent.state;
            x_eventdata.button       = 0x00;
            x_eventdata.same_screen  = x_dmevent.same_screen;
            x_eventdata.device_state = x_dmevent.device_state;
            x_eventdata.axes_count   = x_dmevent.axes_count;
            x_eventdata.first_axis   = x_dmevent.first_axis;
            x_eventdata.axis_data[0] = x_dmevent.axis_data[0];
            x_eventdata.axis_data[1] = x_dmevent.axis_data[1];
            x_eventdata.axis_data[2] = x_dmevent.axis_data[2];
            x_eventdata.axis_data[3] = x_dmevent.axis_data[3];
            x_eventdata.axis_data[4] = x_dmevent.axis_data[4];
            x_eventdata.axis_data[5] = x_dmevent.axis_data[5];
        }
        else
        {
            x_eventdata = *( XDeviceButtonEvent* )&x_event;
        }
        
        bqt::window_event w_event;
        w_event.type = NONE;
        
        Screen* x_screen = DefaultScreenOfDisplay( x_event.xany.display );

        int x_screen_px[ 2 ];
        float x_screen_mm[ 2 ];
        float x_screen_res[ 2 ];

        x_screen_px[ 0 ] = WidthOfScreen( x_screen );
        x_screen_px[ 1 ] = HeightOfScreen( x_screen );
        x_screen_mm[ 0 ] = WidthMMOfScreen( x_screen );
        x_screen_mm[ 1 ] = HeightMMOfScreen( x_screen );
        x_screen_res[ 0 ] = x_screen_mm[ 0 ] / x_screen_px[ 0 ];
        x_screen_res[ 1 ] = x_screen_mm[ 1 ] / x_screen_px[ 1 ];
        
        if( x_eventtype == BUTTON_RELEASE
            && x_eventdata.button > Button3 )                                   // Ignore scroll button up events
        {
            return;
        }
        
        x_input_detail& device_detail( x_input_devices[ x_eventdata.deviceid ] );
        
        if( x_eventtype == MOTION )
            w_event.type = STROKE;
        else
        {
            switch( x_eventdata.button )
            {
                case Button1:
                case Button2:
                case Button3:
                    w_event.type = STROKE;
                    break;
                case Button4:
                case Button5:
                case ( Button5 + 1 ):                                           // aka Button6
                case ( Button5 + 2 ):                                           // aka Button7
                    w_event.type = SCROLL;
                    break;
                default:
                    break;                                                      // Ignore all other buttons for now
            }
        }
        
        switch( w_event.type )
        {
            case STROKE:
                {
                    w_event.stroke.dev_id = device_detail.x_devid;
                    
                    w_event.stroke.click = 0x00;
                    
                    if( device_detail.relative )
                    {
                        if( warn_relative )
                        {
                            ff::write( bqt_out, "Warning: Devices in relative mode will not have sub-pixel accuracy\n" );
                            warn_relative = false;
                        }
                        
                        if( x_eventdata.state & Button1Mask )                   // Button1 = left click
                            w_event.stroke.click |= CLICK_PRIMARY;
                        if( x_eventdata.state & Button3Mask )                   // Button3 = right click
                            w_event.stroke.click |= CLICK_SECONDARY;
                        if( x_eventdata.state & Button2Mask )                   // Button2 = middle click
                            w_event.stroke.click |= CLICK_ALT;
                        
                        w_event.stroke.position[ 0 ] = ( float )x_eventdata.x;
                        w_event.stroke.position[ 1 ] = ( float )x_eventdata.y;
                        
                        w_event.stroke.pressure = 0.0f;                         // Pressure may be set later
                        
                        w_event.stroke.tilt[ 0 ] = 0.0f;
                        w_event.stroke.tilt[ 1 ] = 0.0f;
                        
                        w_event.stroke.wheel = 0.0f;
                        w_event.stroke.rotation = 0.0f;
                    }
                    else
                    {
                        #warning Assuming any absolute device is a tablet with at least 6 axes
                        
                        if( x_eventdata.state & Button1Mask )
                            w_event.stroke.click |= CLICK_PRIMARY;
                        
                        w_event.stroke.position[ 0 ] = ( ( float )x_eventdata.axis_data[ 0 ]
                                                         / ( float )device_detail.axes[ 0 ].max_value )
                                                       * ( float )x_screen_px[ 0 ];
                        w_event.stroke.position[ 1 ] = ( ( float )x_eventdata.axis_data[ 1 ]
                                                         / ( float )device_detail.axes[ 1 ].max_value )
                                                       * ( float )x_screen_px[ 1 ];
                        
                        if( device_detail.type == TOUCH_STYLUS )                // Unpressured touch events report a 0 pressure
                            w_event.stroke.pressure = 1.0f;
                        else
                            w_event.stroke.pressure = ( float )x_eventdata.axis_data[ 2 ]
                                                      / ( float )device_detail.axes[ 2 ].max_value;
                        
                        // TODO: Account for | min_value | > | max_value |
                        
                        w_event.stroke.tilt[ 0 ] = ( float )x_eventdata.axis_data[ 3 ]
                                                   / ( float )device_detail.axes[ 3 ].max_value;
                        w_event.stroke.tilt[ 1 ] = ( float )x_eventdata.axis_data[ 4 ]
                                                   / ( float )device_detail.axes[ 4 ].max_value;
                        
                        // Note that for at least some styluses with no wheel,
                        // the Linux Wacom driver reports the wheel at minimum
                        // value rather than rest value (0.0) - however there
                        // doesn't seem to be a way to tell if this value is
                        // being used this way or if it's real input.
                        if( device_detail.type == AIRBRUSH_STYLUS )             // Wacom 6th axis' meaning depends on device
                        {
                            w_event.stroke.wheel = ( float )x_eventdata.axis_data[ 5 ]
                                                   / ( float )device_detail.axes[ 5 ].max_value;
                            w_event.stroke.rotation = 0.0f;
                        }
                        else
                        {
                            w_event.stroke.wheel = 0.0f;
                            w_event.stroke.rotation = ( float )x_eventdata.axis_data[ 5 ]
                                                      / ( float )device_detail.axes[ 5 ].max_value;
                        }
                    }
                    
                    if( x_eventtype != MOTION )
                    {
                        switch( x_eventdata.button )                            // Fix for XInput not reporting the pressed button in the mask, but reporting it
                                                                                // for releases (complete opposite of what we'd want)
                        {
                            case Button1:
                                if( x_eventtype == BUTTON_PRESS )
                                    w_event.stroke.click |= CLICK_PRIMARY;      // Add click, which isn't there
                                else
                                    w_event.stroke.click &= ~CLICK_PRIMARY;     // Remove click, which is already there
                                break;
                            case Button3:
                                if( x_eventtype == BUTTON_PRESS )
                                    w_event.stroke.click |= CLICK_SECONDARY;
                                else
                                    w_event.stroke.click &= ~CLICK_SECONDARY;
                                break;
                            case Button2:
                                if( x_eventtype == BUTTON_PRESS )
                                    w_event.stroke.click |= CLICK_ALT;
                                else
                                    w_event.stroke.click &= ~CLICK_ALT;
                                break;
                            default:
                                break;                                          // Only interested in first three
                        }
                    }
                    if( w_event.stroke.click & CLICK_PRIMARY )                  // Set pressure & stylus type here now that we know the click
                    {
                        if( device_detail.relative )                            // Pressure is already set for tablets
                            w_event.stroke.pressure = 1.0f;
                        
                        switch( device_detail.type )
                        {
                            case PEN_STYLUS:
                            case AIRBRUSH_STYLUS:
                            case TOUCH_STYLUS:
                            default:
                                w_event.stroke.click |= CLICK_PRIMARY;
                                break;
                            case ERASER_STYLUS:
                                w_event.stroke.click |= CLICK_ERASE;
                                break;
                        }
                    }
                    
                    w_event.stroke.shift = ( bool )( x_eventdata.state & ShiftMask   );
                    w_event.stroke.ctrl  = ( bool )( x_eventdata.state & ControlMask );
                    w_event.stroke.alt   = ( bool )( x_eventdata.state & Mod1Mask    );
                    w_event.stroke.super = ( bool )( x_eventdata.state & Mod4Mask    );
                    
                    #ifdef PLATFORM_MACOSX
                    w_event.stroke.cmd = w_event.stroke.super;
                    #else
                    w_event.stroke.cmd = w_event.stroke.ctrl;
                    #endif
                    
                    stroke_waypoint& prev_waypoint( prev_strokes[ device_detail.x_devid ] );
                    w_event.stroke.prev_pos[ 0 ] = prev_waypoint.position[ 0 ];
                    w_event.stroke.prev_pos[ 1 ] = prev_waypoint.position[ 1 ];
                    prev_waypoint = w_event.stroke;                             // Update previous stroke
                }
                break;
            case SCROLL:
                {
                    w_event.scroll.position[ 0 ] = x_eventdata.x;               // XDeviceButtonEvent::x & x_root seemt to report the same value
                    w_event.scroll.position[ 1 ] = x_eventdata.y;
                    
                    w_event.scroll.amount[ 0 ] = 0.0f;                          // Initialize to 0.0
                    w_event.scroll.amount[ 1 ] = 0.0f;
                    switch( x_eventdata.button )
                    {
                    case Button4:                                               // Scroll wheel up
                        w_event.scroll.amount[ 1 ] =  1.0f * bqt::getWheelScrollDistance();
                        break;
                    case Button5:                                               // Scroll wheel down
                        w_event.scroll.amount[ 1 ] = -1.0f * bqt::getWheelScrollDistance();
                        break;
                    case ( Button5 + 1 ):                                       // Scroll wheel left
                        w_event.scroll.amount[ 0 ] =  1.0f * bqt::getWheelScrollDistance();
                        break;
                    case ( Button5 + 2 ):                                       // Scroll wheel right
                        w_event.scroll.amount[ 0 ] = -1.0f * bqt::getWheelScrollDistance();
                        break;
                    default:
                        break;                                                  // Realistically we'll never get here
                    }
                    
                    w_event.scroll.shift = ( bool )( x_eventdata.state & ShiftMask   );
                    w_event.scroll.ctrl  = ( bool )( x_eventdata.state & ControlMask );
                    w_event.scroll.alt   = ( bool )( x_eventdata.state & Mod1Mask    );
                    w_event.scroll.super = ( bool )( x_eventdata.state & Mod4Mask    );
                    
                    #ifdef PLATFORM_MACOSX
                    w_event.scroll.cmd = w_event.scroll.super;
                    #else
                    w_event.scroll.cmd = w_event.scroll.ctrl;
                    #endif
                }
                break;
            case NONE:
            default:
                break;                                                          // Nothing to do
        }
        
        if( w_event.type != NONE )
            target.acceptEvent( w_event );
    }
}


