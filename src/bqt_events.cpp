/* 
 * bqt_events.cpp
 * 
 * Clicks & strokes are both converted to stroke events & passed to windows,
 * as it is entirely up to the window to check the event position(s) and thus to
 * decide whether it is a drag or a click.
 * 
 * Also implements setQuitFlag() and getQuitFlag() from bqt_platform.h
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_events.hpp"

#include <map>
#include <vector>

#include <SDL2/SDL_events.h>

#include "bqt_taskexec.hpp"
#include "bqt_windowmanagement.hpp"
#include "bqt_window.hpp"
#include "bqt_windowevent.hpp"
#include "bqt_platform.h"
#include "bqt_exception.hpp"
#include "bqt_launchargs.hpp"
#include "bqt_mutex.hpp"
#include "bqt_keycode.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    std::map< Window, bqt::window::manipulate* > window_manipulates;            // Waiting room so we don't submit more than we need
                                                                                // TODO: Remove when we overload new for manipulates
    
    // INPUT DEVICES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef PLATFORM_XWS_GNUPOSIX

    #define DEVICE_CLASS_LIST_START_LENGTH  16

    enum pen_type
    {
        INVALID,
        PEN_STYLUS,
        AIRBRUSH_STYLUS,
        ERASER_STYLUS,
        TOUCH_STYLUS
    };
    
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
    
    #endif
    
    // X QUIT HANDLING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef PLATFORM_XWS_GNUPOSIX
    
    bqt::mutex quit_mutex;
    bool quit_flag = false;
    
    #endif
    
    // EVENT HANDLERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    void handleKeyEvent( XEvent& x_event )
    {
        XKeyEvent& x_kevent( *( ( XKeyEvent* )&x_event ) );
        
        bqt_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        bqt::window* target_window = &bqt::getWindow( platform_window );
        
        if( target_window == NULL )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Got key event for unknown window, using active window\n" );
            target_window = bqt::getActiveWindow();
        }
        
        if( target_window == NULL )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Got key event with no active window, ignoring\n" );
        }
        else
        {
            bqt::window_event w_event;
            w_event.type = bqt::KEYCOMMAND;
            
            switch( x_kevent.type )
            {
                case KeyPress:
                    w_event.key.up = false;
                    break;
                case KeyRelease:
                    w_event.key.up = true;
                    break;
                default:
                    throw bqt::exception( "handleKeyEvent(): Key event neither up nor down" );
            }
            
            w_event.key.key = bqt::convertPlatformKeycode( XLookupKeysym( &x_kevent, 0 ) );
            
            if( w_event.key.key != bqt::KEY_INVALID )                           // Simply ignore invalid keys
            {
                w_event.key.shift = ( bool )( x_kevent.state & ShiftMask );
                w_event.key.ctrl  = ( bool )( x_kevent.state & ControlMask );
                w_event.key.alt   = ( bool )( x_kevent.state & Mod1Mask );
                w_event.key.super = ( bool )( x_kevent.state & Mod4Mask );      // Run xmodmap to find these on a given system
                
                #ifdef PLATFORM_MACOSX
                w_event.key.cmd = w_event.key.super;
                #else
                w_event.key.cmd = w_event.key.ctrl;
                #endif
                
                ff::write( bqt_out,
                           "Key command ",
                           bqt::getKeyCommandString( w_event.key ),
                           ( w_event.key.up ? " (Up)" : " (Down)" ),
                           "\n" );
                
                target_window -> acceptEvent( w_event );
            }
        }
    }
    
    void handleTouchEvent( XEvent& x_event )
    {
        ff::write( bqt_out, "Touch event\n" );
    }
    
    void handleMotionEvent( XEvent& x_event )
    {
        bqt::window_event w_event;
        w_event.type = bqt::NONE;
        w_event.type = bqt::COMMAND;
        
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
            break;
        case ButtonRelease:
            break;
        case MotionNotify:
            break;
        default:
            {
                // lol at least we want sub-pixel positions anyways
                // https://bugs.freedesktop.org/show_bug.cgi?id=41757
                
                XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&x_event ) );
                
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
                
                for( int i = 0; i < x_tablet_devices.size(); ++i )
                {
                    if( x_tablet_devices[ i ].x_motioneventtype == x_event.type
                        && x_tablet_devices[ i ].x_devid == x_dmevent.deviceid )
                    {
                        w_event.type = bqt::STROKE;
                        
                        if( x_tablet_devices[ i ].type == ERASER_STYLUS )
                            w_event.stroke.click = CLICK_ERASE;
                        else
                            w_event.stroke.click = CLICK_PRIMARY;               // TODO: Figure out other click types from modifier keys
                        
                        w_event.key.shift = ( bool )( x_dmevent.state & ShiftMask );
                        w_event.key.ctrl  = ( bool )( x_dmevent.state & ControlMask );
                        w_event.key.alt   = ( bool )( x_dmevent.state & Mod1Mask );
                        w_event.key.super = ( bool )( x_dmevent.state & Mod4Mask );
                        
                        #ifdef PLATFORM_MACOSX
                        w_event.key.cmd = w_event.key.super;
                        #else
                        w_event.key.cmd = w_event.key.ctrl;
                        #endif
                        
                        // TODO: support less-than-fullscreen tablet surfaces
                        w_event.stroke.position[ 0 ] = ( ( float )x_dmevent.axis_data[ 0 ]
                                                         / ( float )x_tablet_devices[ i ].axes[ 0 ].max_value )
                                                       * ( float )x_screen_px[ 0 ];
                        w_event.stroke.position[ 1 ] = ( ( float )x_dmevent.axis_data[ 1 ]
                                                         / ( float )x_tablet_devices[ i ].axes[ 1 ].max_value )
                                                       * ( float )x_screen_px[ 1 ];
                        
                        if( x_tablet_devices[ i ].type == TOUCH_STYLUS )        // We assume no touch pressure right now
                            w_event.stroke.pressure = 1.0f;
                        else
                            w_event.stroke.pressure = ( float )x_dmevent.axis_data[ 2 ] / ( float )x_tablet_devices[ i ].axes[ 2 ].max_value;
                        
                        // TODO: Account for | min_value | > | max_value |
                        
                        w_event.stroke.tilt[ 0 ] = ( float )x_dmevent.axis_data[ 3 ] / ( float )x_tablet_devices[ i ].axes[ 3 ].max_value;
                        w_event.stroke.tilt[ 1 ] = ( float )x_dmevent.axis_data[ 4 ] / ( float )x_tablet_devices[ i ].axes[ 4 ].max_value;
                        
                        if( x_tablet_devices[ i ].type == AIRBRUSH_STYLUS )     // Wacom 6th axis' meaning depends on device
                        {
                            w_event.stroke.wheel = ( float )x_dmevent.axis_data[ 5 ] / ( float )x_tablet_devices[ i ].axes[ 5 ].max_value;
                            w_event.stroke.rotation = 0.0;
                        }
                        else
                        {
                            w_event.stroke.wheel = 0.0;
                            w_event.stroke.rotation = ( float )x_dmevent.axis_data[ 5 ] / ( float )x_tablet_devices[ i ].axes[ 5 ].max_value;
                        }
                        
                        break;
                    }
                }
            }
            break;
        }
        
        if( w_event.type != bqt::NONE )
            target -> acceptEvent( w_event );
    }
    
    void handleTextEvent( XEvent& x_event )
    {
        // http://www.x.org/releases/X11R7.6/doc/libX11/specs/XIM/xim.html
    }
    
    void handleWindowEvent( XEvent& x_event )
    {
        using namespace bqt;
        
        bqt_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        bqt::window& bqt_window( getWindow( platform_window ) );
        
        if( isRegisteredWindow( platform_window ) )
        {
            if( !window_manipulates.count( platform_window.x_window ) )
                window_manipulates[ platform_window.x_window ] = new window::manipulate( &bqt_window );
        }
        else
        {
            if( getDevMode() )
                ff::write( bqt_out, "X window event received for unregistered window, ignoring\n" );
            
            return;
        }
        
        window::manipulate* current_manip = window_manipulates[ platform_window.x_window ];
        
        switch( x_event.type )
        {
        // case :  // Shown
        //     break;
        case Expose:
            if( x_event.xexpose.count != 0 )
                current_manip -> redraw();
            break;
        case ConfigureRequest:
            // ff::write( bqt_out, "ConfigureRequest\n" );
            // XConfigureRequestEvent& x_cfg_event( x_event.xconfigurerequest );
            break;
        case ConfigureNotify:
            {
                XConfigureEvent& x_cfg_event( x_event.xconfigure );
                
                // if( x_cfg_event.value_mask & CWX || x_cfg_event.value_mask & CWY )
                if( x_cfg_event.x !=bqt_window.getPosition().first || x_cfg_event.y !=bqt_window.getPosition().second )
                    current_manip -> setPosition( x_cfg_event.x, x_cfg_event.y );
                
                // if( x_cfg_event.value_mask & CWWidth || x_cfg_event.value_mask & CWHeight )
                if( x_cfg_event.width != bqt_window.getDimensions().first || x_cfg_event.height != bqt_window.getDimensions().second )
                {
                    if( x_cfg_event.width < 1 || x_cfg_event.height < 1 )       // Trust no one
                        throw exception( "handleWindowEvent(): Width or height not within limits" );
                    
                    // ff::write( bqt_out, "Setting dimensions\n" );
                    current_manip -> setDimensions( x_cfg_event.width, x_cfg_event.height );
                }
            }
            break;
        case MapNotify:
        case MapRequest:
            current_manip -> makeActive();
        case VisibilityNotify:
            current_manip -> restore();
            break;
        case ClientMessage:
            // http://tronche.com/gui/x/icccm/sec-4.html#s-4.2.8.1
            if( x_event.xclient.data.l[ 0 ] == XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False ) )
                current_manip -> close();
            break;
        // case :  // Minimize
        //     current_manip -> minimize();
        //     break;
        // case :  // Maximize
        //     current_manip -> maximize();
        //     break;
        // case :  // Mouse Focus
        //     break;
        // case :  // Keyboard Focus
        //     break;
        default:
            if( getDevMode() )
                ff::write( bqt_out, "Received unrecognized X window event, ignoring\n" );
            break;
        }
    }
    
    void openTabletDevices()
    {
        using namespace bqt;
        
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
        
        // Much thanks to
        // http://mobileim.googlecode.com/svn/MeeGo/meegoTraning/MeeGo_Traing_Doc_0119/meego_trn/meego-quality-assurance-mcts/mcts-blts/blts-x11/src/xinput_tests.c
        // for demonstrating how to use XDeviceInfo.
        
        XDeviceInfo* x_dev_info = XListInputDevices( x_display, &x_device_count );
        {    
            if( x_dev_info == NULL )
                throw exception( "openTabletDevices(): Failed to get list of X devices" );
            
            for( int i = 0; i < x_device_count; i++ )
            {
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
                            ff::write( bqt_out, "unknown class\n" );
                            break;                                              // Ignore default
                        }
                        x_any = ( XAnyClassPtr )( ( char* )x_any + x_any -> length );   // THIS IS HOW YOU'RE SUPPOSED TO DO IT?
                    }
                    
                    DeviceMotionNotify( detail.x_device,
                                        detail.x_motioneventtype,
                                        x_eventclass_list[ x_eventclass_count ] );
                    ++x_eventclass_count;                                       // DeviceMotionNotify() is a macro, so increment outside
                    
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
                // ff::write( bqt_out, "Testing XCloseDevice\n" );
                // XCloseDevice( x_display, new_devices[ i ].x_device );
                ff::write( bqt_out, "Leaking device \"", new_devices[ i ].name, "\" (unplugged?)\n" );
            }
    }
    
    void closeTabletDevices()
    {
        using namespace bqt;
        
        Display* x_display = getXDisplay();
        
        for( int i = 0; i < x_tablet_devices.size(); i++ )
        {
            XCloseDevice( x_display, x_tablet_devices[ i ].x_device );
            
            if( getDevMode() )
                ff::write( bqt_out, "Closed device \"", x_tablet_devices[ i ].name, "\"\n" );
        }
        
        x_tablet_devices.clear();
    }
    
    #endif
}

/******************************************************************************//******************************************************************************/

#if defined PLATFORM_XWS_GNUPOSIX

void setQuitFlag()
{
    bqt::scoped_lock slock( quit_mutex );
    quit_flag = true;
}
bool getQuitFlag()
{
    bqt::scoped_lock slock( quit_mutex );
    return quit_flag;
}

namespace bqt
{
    bool HandleEvents_task::execute( task_mask* caller_mask )
    {
        if( getQuitFlag() )
        {
            if( getDevMode() )
                ff::write( bqt_out, "Quitting...\n" );
            closeTabletDevices();
            closeAllWindows();
            submitTask( new StopTaskSystem_task() );
        }
        else
        {
            XEvent x_event;
            Display* x_display = getXDisplay();
            
            {                                                                   // Check to see if we want to refresh device list
                int new_device_count;
                XFreeDeviceList( XListInputDevices( x_display, &new_device_count ) );   // No clear way of getting just the count
                
                if( new_device_count != x_device_count )
                {
                    openTabletDevices();
                }
            }
            
            for( int queue_size = XEventsQueued( x_display, QueuedAfterFlush ); // AKA XPending( x_display )
                 queue_size > 0;
                 --queue_size )                                                 // Yay we can guarantee termination
            {
                XNextEvent( x_display, &x_event );
                
                switch( x_event.type )
                {
                // case DevicePresenceNotify:
                //     ff::write( bqt_out, "DevicePresenceNotify\n" );
                    // XDeviceListChangeEvent: http://www.x.org/wiki/XInputSpec/
                case KeyPress:
                case KeyRelease:
                    handleKeyEvent( x_event );
                    break;
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
                    handleMotionEvent( x_event );
                    break;
                case Expose:
                case ConfigureRequest:
                case ConfigureNotify:
                case MapNotify:
                case MapRequest:
                case ClientMessage:
                case VisibilityNotify:
                    handleWindowEvent( x_event );
                    break;
                case DestroyNotify:
                case CreateNotify:
                case UnmapNotify:
                case ReparentNotify:
                case GravityNotify:
                case EnterNotify:
                case LeaveNotify:
                case FocusIn:
                case FocusOut:
                case GraphicsExpose:
                case NoExpose:
                case CirculateNotify:
                case CirculateRequest:
                case SelectionClear:
                case SelectionRequest:
                case SelectionNotify:
                case ColormapNotify:
                case PropertyNotify:
                    break;                                                      // Ignore, for now
                default:
                    handleMotionEvent( x_event );                               // DeviceMotion events have a dynamic type
                    break;
                }
            }
            
            for( std::map< Window, bqt::window::manipulate* >::iterator iter = window_manipulates.begin();
                 iter != window_manipulates.end();
                 ++iter )
            {
                submitTask( iter -> second );
            }
            window_manipulates.clear();
        }
        
        return false;                                                           // Requeue instead of submitting a new copy
    }
}

#else

#error "Events not implemented on non-X platforms"

#endif


