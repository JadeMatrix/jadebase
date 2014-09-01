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
#include <vector>
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
    
    // TABLET INFO /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    enum pen_type
    {
        INVALID,
        PEN_STYLUS,
        AIRBRUSH_STYLUS,
        ERASER_STYLUS,
        TOUCH_STYLUS
    };
    
    struct x_tablet_dev_detail
    {
        std::string name;                                                       // std::string instead of const char* so we copy the string easily
        pen_type type;
        
        XAxisInfo axes[ 6 ];
        
        XID x_devid;
        XDevice* x_device;
        int x_motioneventtype;
    };
    
    std::vector< x_tablet_dev_detail > x_tablet_devices;
    int x_device_count = 0;
    
    pen_type getWacomTabletType( const char* name )                             // The Linux Wacom driver reports names as "Wacom ... *type*"
    {
        std::string name_str( name );
        
        for( int i = 0; i < 5; ++i )
        {
            if( name_str[ i ] != "Wacom"[ i ] )
                return INVALID;
        }
        
        for( int i = name_str.length() - 1; i >= 0; --i )
        {
            if( name_str[ i ] == ' ' )
            {
                std::string type_str = name_str.substr( i + 1 );                // i + 1 so we discard the space
                
                if( type_str == "stylus" )
                    return PEN_STYLUS;
                if( type_str == "eraser" )
                    return ERASER_STYLUS;
                if( type_str == "touch" )
                    return TOUCH_STYLUS;
                // if( type_str == "" )
                //     return AIRBRUSH_STYLUS;
                // if( type_str == "pad" )
                //     return _STYLUS;
                
                break;
            }
        }
        
        return INVALID;
    }
    
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

#define DEVICE_CLASS_LIST_START_LENGTH  16

namespace bqt
{
    void openInputDevices()
    {
        scoped_lock< mutex > slock( idev_mutex );
        
        ff::write( bqt_out, "Opening input devices\n" );
        
        // for stroke_waypoint collecting:
        // http://www.cplusplus.com/reference/vector/vector/reserve/
        
        Display* x_display = getXDisplay();
        int x_screen = DefaultScreen( x_display );
        
        std::vector< x_tablet_dev_detail > new_devices;                         // For swapping with global list
        
        int x_eventclass_size = DEVICE_CLASS_LIST_START_LENGTH;                 // Max devices is 128 so int is OK
        XEventClass* x_eventclass_list = new XEventClass[ x_eventclass_size ];  // memcpy() if we need more
        // XEventClass* x_eventclass_list = ( XEventClass* )calloc( x_eventclass_size, sizeof( XEventClass ) );
        int x_eventclass_count = 0;
        
        float x_screen_res[ 2 ];
        {
            Screen* x_screen_ptr = DefaultScreenOfDisplay( x_display );
            x_screen_res[ 0 ] = WidthMMOfScreen( x_screen_ptr ) / WidthOfScreen( x_screen_ptr );
            x_screen_res[ 1 ] = HeightMMOfScreen( x_screen_ptr ) / HeightOfScreen( x_screen_ptr );
        }
        
        if( !prev_motion_events.count( dummy_idevid ) )
            prev_motion_events[ dummy_idevid ] = initial_waypoint;
        
        // Much thanks to
        // http://mobileim.googlecode.com/svn/MeeGo/meegoTraning/MeeGo_Traing_Doc_0119/meego_trn/meego-quality-assurance-mcts/mcts-blts/blts-x11/src/xinput_tests.c
        // for demonstrating how to use XDeviceInfo.
        
        XDeviceInfo* x_dev_info = XListInputDevices( x_display, &x_device_count );
        {    
            if( x_dev_info == NULL )
                throw exception( "openTabletDevices(): Failed to get list of X devices" );
            
            ff::write( bqt_out, XI_MOUSE, " = ", ( unsigned long )XInternAtom( x_display, XI_MOUSE, true ), "\n" );
            ff::write( bqt_out, XI_TABLET, " = ", ( unsigned long )XInternAtom( x_display, XI_TABLET, true ), "\n" );
            ff::write( bqt_out, XI_KEYBOARD, " = ", ( unsigned long )XInternAtom( x_display, XI_KEYBOARD, true ), "\n" );
            ff::write( bqt_out, XI_TOUCHSCREEN, " = ", ( unsigned long )XInternAtom( x_display, XI_TOUCHSCREEN, true ), "\n" );
            ff::write( bqt_out, XI_TOUCHPAD, " = ", ( unsigned long )XInternAtom( x_display, XI_TOUCHPAD, true ), "\n" );
            ff::write( bqt_out, XI_BUTTONBOX, " = ", ( unsigned long )XInternAtom( x_display, XI_BUTTONBOX, true ), "\n" );
            ff::write( bqt_out, XI_BARCODE, " = ", ( unsigned long )XInternAtom( x_display, XI_BARCODE, true ), "\n" );
            ff::write( bqt_out, XI_TRACKBALL, " = ", ( unsigned long )XInternAtom( x_display, XI_TRACKBALL, true ), "\n" );
            ff::write( bqt_out, XI_QUADRATURE, " = ", ( unsigned long )XInternAtom( x_display, XI_QUADRATURE, true ), "\n" );
            ff::write( bqt_out, XI_ID_MODULE, " = ", ( unsigned long )XInternAtom( x_display, XI_ID_MODULE, true ), "\n" );
            ff::write( bqt_out, XI_ONE_KNOB, " = ", ( unsigned long )XInternAtom( x_display, XI_ONE_KNOB, true ), "\n" );
            ff::write( bqt_out, XI_NINE_KNOB, " = ", ( unsigned long )XInternAtom( x_display, XI_NINE_KNOB, true ), "\n" );
            ff::write( bqt_out, XI_KNOB_BOX, " = ", ( unsigned long )XInternAtom( x_display, XI_KNOB_BOX, true ), "\n" );
            ff::write( bqt_out, XI_SPACEBALL, " = ", ( unsigned long )XInternAtom( x_display, XI_SPACEBALL, true ), "\n" );
            ff::write( bqt_out, XI_DATAGLOVE, " = ", ( unsigned long )XInternAtom( x_display, XI_DATAGLOVE, true ), "\n" );
            ff::write( bqt_out, XI_EYETRACKER, " = ", ( unsigned long )XInternAtom( x_display, XI_EYETRACKER, true ), "\n" );
            ff::write( bqt_out, XI_CURSORKEYS, " = ", ( unsigned long )XInternAtom( x_display, XI_CURSORKEYS, true ), "\n" );
            ff::write( bqt_out, XI_FOOTMOUSE, " = ", ( unsigned long )XInternAtom( x_display, XI_FOOTMOUSE, true ), "\n" );
            ff::write( bqt_out, XI_JOYSTICK, " = ", ( unsigned long )XInternAtom( x_display, XI_JOYSTICK, true ), "\n" );
            
            for( int i = 0; i < x_device_count; i++ )
            {
                ff::write( bqt_out,
                           "Found a device: \"",
                           x_dev_info[ i ].name,
                           "\" as id ",
                           x_dev_info[ i ].id,
                           ", it's a ",
                           // type_name,
                           x_dev_info[ i ].type ? XGetAtomName( x_display, x_dev_info[ i ].type ) : "????",
                           " (",
                           ( unsigned long )x_dev_info[ i ].type,
                           ")\n" );
                
                pen_type dev_pen_type = getWacomTabletType( x_dev_info[ i ].name );
                
                if( dev_pen_type != INVALID )
                {
                    // Declare up here because of gotos
                    x_tablet_dev_detail detail;                                 // I give up, these names are getting too long
                    XAnyClassPtr x_any;
                    
                    for( std::vector< x_tablet_dev_detail >::iterator iter = x_tablet_devices.begin();
                         iter != x_tablet_devices.end();
                         ++iter )
                    {
                        if( iter -> x_devid == x_dev_info[ i ].id )             // Already open
                        {
                            new_devices.push_back( *iter );
                            x_tablet_devices.erase( iter );
                            
                            goto next_device;
                        }
                    }
                    
                    detail.name = x_dev_info[ i ].name;
                    detail.type = dev_pen_type;
                    detail.x_devid = x_dev_info[ i ].id;
                    detail.x_device = XOpenDevice( x_display, detail.x_devid  );
                    
                    if( detail.x_device == NULL )
                        ff::write( bqt_out, "Warning: Found device \"", detail.name, "\" but could not open\n" );
                    else
                        if( getDevMode() )
                            ff::write( bqt_out, "Opened device \"", detail.name, "\" as id ", detail.x_devid, ", loc ", ( void* )detail.x_device, "\n" );
                    
                    x_any = ( XAnyClassPtr )x_dev_info[ i ].inputclassinfo;
                    for( int k = 0; k < x_dev_info[ i ].num_classes; ++k )
                    {
                        if( sizeof( XID ) != sizeof( long ) )                   // TODO: Find a way to make this a compile-time error
                            throw exception( "openTabletDevices(): X class 'class' member hack does not work" );
                        // switch( x_any -> class )                             // OH NO YOU CAN'T DO THIS IS C++ BABY
                        switch( *( ( long* )x_any  ) )                          // 'class' should be sizeof( long ) & the first element of XAnyClass
                        {
                        case KeyClass:
                        case ButtonClass:
                            break;                                              // Ignore here
                        case ValuatorClass:
                            {
                                XValuatorInfo* xval_info = ( XValuatorInfo* )x_any;
                                
                                if( xval_info -> mode != Absolute )
                                {
                                    ff::write( bqt_out, "Warning: Tablet device found in relative mode, ignoring\n" );
                                    goto next_device;
                                }
                                
                                if( xval_info -> num_axes != 6 )                // Throw exception until its supported (if necessary)
                                    throw exception( "openTabletDevices(): Tablet device with axis count != 6 found\n" );
                                
                                for( int axis = 0; axis < xval_info -> num_axes; ++axis )
                                    detail.axes[ axis ] = xval_info -> axes[ axis ];
                            }
                            break;
                        default:
                            break;                                              // Ignore default
                        }
                        x_any = ( XAnyClassPtr )( ( char* )x_any + x_any -> length );   // THIS IS HOW YOU'RE SUPPOSED TO DO IT?
                    }
                    
                    DeviceMotionNotify( detail.x_device,
                                        detail.x_motioneventtype,
                                        x_eventclass_list[ x_eventclass_count ] );
                    ++x_eventclass_count;                                       // DeviceMotionNotify() is a macro; we have to increment outside
                    
                    if( x_eventclass_count >= x_eventclass_size )
                    {
                        ff::write( bqt_out, "Warning: Max tablet devices opened, ignoring rest\n" );
                        break;
                        
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
                    
                    if( 1000 / detail.axes[ 0 ].resolution                      // If the device l/mm is larger than the smallest screen l/mm
                        > ( x_screen_res[ 0 ] < x_screen_res[ 1 ] ? x_screen_res[ 0 ] : x_screen_res[ 1 ] ) )
                    {
                        ff::write( bqt_out, "Warning: Device \"", detail.name, "\" has a larger resolution than the screen\n" );
                    }
                    
                    bqt_platform_idevid_t platform_devid;
                    
                    platform_devid = detail.x_devid;
                    
                    if( !prev_motion_events.count( platform_devid ) )           // Add previous up motion events
                    {
                        prev_motion_events[ platform_devid ] = initial_waypoint;
                        prev_motion_events[ platform_devid ].dev_id = platform_devid;
                    }
                    
                    new_devices.push_back( detail );
                    
                next_device:
                    NULL;
                }
            }
            
            XSelectExtensionEvent( x_display,
                                   RootWindow( x_display, x_screen ),
                                   x_eventclass_list,
                                   x_eventclass_count );
        }
        XFreeDeviceList( x_dev_info );
        
        if( getDevMode() && new_devices.size() == 0 )
            ff::write( bqt_out, "No tablet devices found\n" );
        
        x_tablet_devices.swap( new_devices );                                   // We have to jump through this hoop because if a device was unplugged while
                                                                                // open, trying to close it will result in an error.  The only option is to leak
                                                                                // the memory.  This project concurs:
                                                                                // https://github.com/Alcaro/minir/blob/master/inputraw-x11-xinput2.c
        if( getDevMode() )
            for( int i = 0; i < new_devices.size(); ++i )
            {
                bqt_platform_idevid_t platform_devid;
                
                platform_devid = new_devices[ i ].x_devid;
                
                if( prev_motion_events.count( platform_devid ) )                // Clean up previous motion events
                    prev_motion_events.erase( platform_devid );
                
                // ff::write( bqt_out, "Testing XCloseDevice\n" );
                // XCloseDevice( x_display, new_devices[ i ].x_device );
                ff::write( bqt_out, "Leaking device \"", new_devices[ i ].name, "\" (unplugged?)\n" );
            }
    }
    void closeInputDevices()
    {
        scoped_lock< mutex > slock( idev_mutex );
        
        ff::write( bqt_out, "Closing input devices\n" );
        
        Display* x_display = getXDisplay();
        
        for( int i = 0; i < x_tablet_devices.size(); i++ )
        {
            bqt_platform_idevid_t platform_devid;
            
            platform_devid = x_tablet_devices[ i ].x_devid;
            
            if( prev_motion_events.count( platform_devid ) )                    // Clean up previous motion events
                prev_motion_events.erase( platform_devid );
            
            XCloseDevice( x_display, x_tablet_devices[ i ].x_device );
            
            if( getDevMode() )
                ff::write( bqt_out, "Closed device \"", x_tablet_devices[ i ].name, "\"\n" );
        }
        
        x_tablet_devices.clear();
    }
    void refreshInputDevices()
    {
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
        
        if( new_device_count != x_device_count )
        {
            openInputDevices();
        }
    }
    
    void handleStrokeEvent( XEvent& x_event )
    {
        bqt::window_event w_event;
                
        // bqt_platform_window_t platform_window;
        // platform_window.x_window = x_event.xany.window;
        // bqt::window* target = bqt::getWindow( platform_window );             // XInput doesn't report any individual window with the event
        bqt::window* target = bqt::getActiveWindow();
        if( target == NULL )
        {
            // if( bqt::getDevMode() )
            //     ff::write( bqt_out, "Warning: Got motion event not matched to a known window, ignoring\n" );
            
            return;
        }
        
        switch( x_event.type )
        {
        case ButtonPress:
            {
                switch( x_event.xbutton.button )
                {
                case Button1:
                case Button2:
                case Button3:
                    {
                        w_event.type = bqt::STROKE;
                        
                        w_event.stroke.dev_id = dummy_idevid;                           
                        
                        w_event.stroke.position[ 0 ] = x_event.xbutton.x_root;
                        w_event.stroke.position[ 1 ] = x_event.xbutton.y_root;
                        
                        w_event.stroke.shift = ( bool )( x_event.xbutton.state & ShiftMask );
                        w_event.stroke.ctrl  = ( bool )( x_event.xbutton.state & ControlMask );
                        w_event.stroke.alt   = ( bool )( x_event.xbutton.state & Mod1Mask );
                        w_event.stroke.super = ( bool )( x_event.xbutton.state & Mod4Mask );
                        
                        #ifdef PLATFORM_MACOSX
                        w_event.stroke.cmd = w_event.stroke.super;
                        #else
                        w_event.stroke.cmd = w_event.stroke.ctrl;
                        #endif
                        
                        // Perhaps use x_event.xbutton.state here instead
                        w_event.stroke.click = 0x00;
                        if( x_event.xbutton.button == Button1 )                 // Button1 = left click
                            w_event.stroke.click |= CLICK_PRIMARY;
                        if( x_event.xbutton.button == Button3 )                 // Button3 = right click
                            w_event.stroke.click |= CLICK_SECONDARY;
                        if( x_event.xbutton.button == Button2 )                 // Button2 = middle click
                            w_event.stroke.click |= CLICK_ALT;
                        
                        w_event.stroke.prev_pos[ 0 ] = -INFINITY;
                        w_event.stroke.prev_pos[ 1 ] = -INFINITY;
                        prev_motion_events[ w_event.stroke.dev_id ] = w_event.stroke;
                    }
                    break;
                case Button4:
                case Button5:
                case ( Button5 + 1 ):   // aka Button6
                case ( Button5 + 2 ):   // aka Button7
                    {
                        w_event.type = bqt::SCROLL;
                        
                        w_event.scroll.position[ 0 ] = x_event.xbutton.x_root;
                        w_event.scroll.position[ 1 ] = x_event.xbutton.y_root;
                        
                        w_event.scroll.amount[ 0 ] = 0.0f;
                        w_event.scroll.amount[ 1 ] = 0.0f;
                        switch( x_event.xbutton.button )
                        {
                            case Button4:                                       // Scroll wheel up
                                w_event.scroll.amount[ 1 ] =  1.0f * bqt::getWheelScrollDistance();
                                break;
                            case Button5:                                       // Scroll wheel down
                                w_event.scroll.amount[ 1 ] = -1.0f * bqt::getWheelScrollDistance();
                                break;
                            case ( Button5 + 1 ):                               // Scroll wheel left
                                w_event.scroll.amount[ 0 ] =  1.0f * bqt::getWheelScrollDistance();
                                break;
                            case ( Button5 + 2 ):                               // Scroll wheel right
                                w_event.scroll.amount[ 0 ] = -1.0f * bqt::getWheelScrollDistance();
                                break;
                            default:
                                break;
                        }
                        
                        w_event.scroll.shift = ( bool )( x_event.xbutton.state & ShiftMask );
                        w_event.scroll.ctrl  = ( bool )( x_event.xbutton.state & ControlMask );
                        w_event.scroll.alt   = ( bool )( x_event.xbutton.state & Mod1Mask );
                        w_event.scroll.super = ( bool )( x_event.xbutton.state & Mod4Mask );
                        
                        #ifdef PLATFORM_MACOSX
                        w_event.scroll.cmd = w_event.scroll.super;
                        #else
                        w_event.scroll.cmd = w_event.scroll.ctrl;
                        #endif
                    }
                    break;
                default:
                    // Ignore
                    break;
                }
            }
            break;
        case ButtonRelease:
            {
                if( x_event.xbutton.button == Button4 || x_event.xbutton.button == Button5 )
                    break;                                                      // Ignore scroll button up events
                
                w_event.type = bqt::STROKE;
                
                w_event.stroke.dev_id = dummy_idevid;                           
                
                w_event.stroke.position[ 0 ] = x_event.xbutton.x_root;
                w_event.stroke.position[ 1 ] = x_event.xbutton.y_root;
                
                w_event.stroke.shift = ( bool )( x_event.xbutton.state & ShiftMask );
                w_event.stroke.ctrl  = ( bool )( x_event.xbutton.state & ControlMask );
                w_event.stroke.alt   = ( bool )( x_event.xbutton.state & Mod1Mask );
                w_event.stroke.super = ( bool )( x_event.xbutton.state & Mod4Mask );
                
                #ifdef PLATFORM_MACOSX
                w_event.stroke.cmd = w_event.stroke.super;
                #else
                w_event.stroke.cmd = w_event.stroke.ctrl;
                #endif
                
                w_event.stroke.click = 0x00;
                // if( x_event.xbutton.button == Button1 )
                //     w_event.stroke.click |= CLICK_PRIMARY;
                // if( x_event.xbutton.button == Button3 )
                //     w_event.stroke.click |= CLICK_SECONDARY;
                // if( x_event.xbutton.button == Button2 )
                //     w_event.stroke.click |= CLICK_ALT;
                
                bqt::stroke_waypoint& prev_waypoint = prev_motion_events[ w_event.stroke.dev_id ];
                w_event.stroke.prev_pos[ 0 ] = prev_waypoint.position[ 0 ];
                w_event.stroke.prev_pos[ 1 ] = prev_waypoint.position[ 1 ];
                prev_motion_events.erase( w_event.stroke.dev_id );
            }
            break;
        case MotionNotify:
            {
                w_event.type = bqt::STROKE;
                
                w_event.stroke.dev_id = dummy_idevid;                           
                
                w_event.stroke.click = 0x00;
                if( x_event.xmotion.state & Button1Mask )
                    w_event.stroke.click |= CLICK_PRIMARY;
                if( x_event.xmotion.state & Button3Mask )
                    w_event.stroke.click |= CLICK_SECONDARY;
                if( x_event.xmotion.state & Button2Mask )
                    w_event.stroke.click |= CLICK_ALT;
                
                w_event.stroke.shift = ( bool )( x_event.xmotion.state & ShiftMask );
                w_event.stroke.ctrl  = ( bool )( x_event.xmotion.state & ControlMask );
                w_event.stroke.alt   = ( bool )( x_event.xmotion.state & Mod1Mask );
                w_event.stroke.super = ( bool )( x_event.xmotion.state & Mod4Mask );
                
                #ifdef PLATFORM_MACOSX
                w_event.stroke.cmd = w_event.stroke.super;
                #else
                w_event.stroke.cmd = w_event.stroke.ctrl;
                #endif
                
                w_event.stroke.position[ 0 ] = x_event.xmotion.x_root;
                w_event.stroke.position[ 1 ] = x_event.xmotion.y_root;
                
                w_event.stroke.pressure = 1.0f;
                
                w_event.stroke.tilt[ 0 ] = 0.0f;
                w_event.stroke.tilt[ 1 ] = 0.0f;
                
                w_event.stroke.rotation = 0.0f;
                
                w_event.stroke.wheel = 0.0f;
                
                bqt::stroke_waypoint& prev_waypoint = prev_motion_events[ w_event.stroke.dev_id ];
                w_event.stroke.prev_pos[ 0 ] = prev_waypoint.position[ 0 ];
                w_event.stroke.prev_pos[ 1 ] = prev_waypoint.position[ 1 ];
                prev_waypoint = w_event.stroke;
            }
            break;
        default:
            {
                // lol at least we want sub-pixel positions anyways
                // https://bugs.freedesktop.org/show_bug.cgi?id=41757
                
                XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&x_event ) );
                
                
            }
            break;
        }
        
        if( w_event.type != bqt::NONE )
            target -> acceptEvent( w_event );
    }
}


