/* 
 * bqt_events.cpp
 * 
 * Clicks & strokes are both converted to stroke events & passed to windows,
 * as it is entirely up to the window to check the event position(s) and thus to
 * decide whether it is a drag or a click.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_platform.h"
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

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    std::map< Window, bqt::window::manipulate* > window_manipulates;            // Waiting room so we don't submit more than we need
    
    // INPUT DEVICES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef PLATFORM_XWS_GNUPOSIX

    enum pen_type
    {
        PEN_STYLUS,
        AIRBRUSH_STYLUS,
        ERASER_STYLUS,
        TOUCH_STYLUS
    };
    
    struct x_tablet_device_name
    {
        const char* name;
        pen_type type;
    } x_tablet_device_names[] = { { "wacomdev1"                                   ,    PEN_STYLUS },
                                  { "wacomdev2"                                   , ERASER_STYLUS },
                                  { "Wacom Serial Penabled 2FG Touchscreen stylus",    PEN_STYLUS },
                                  { "Wacom Serial Penabled 2FG Touchscreen eraser", ERASER_STYLUS },
                                  { "Wacom Serial Penabled 2FG Touchscreen touch" ,  TOUCH_STYLUS } };
    
    struct x_tablet_dev_detail
    {
        const char* name;
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
        ff::write( bqt_out, "Key event\n" );
    }
    // void handleKeyEvent( SDL_Event& sdl_event )
    // {
    //     // https://wiki.libsdl.org/SDL_KeyboardEvent
    //     // https://wiki.libsdl.org/SDL_Keysym
    //     // https://wiki.libsdl.org/SDL_Keymod
    //     switch( sdl_event.type )
    //     {
    //     case SDL_KEYDOWN:
    //         break;
    //     case SDL_KEYUP:
    //         break;
    //     default:
    //         break;
    //     }
    // }
    
    void handleTouchEvent( XEvent& x_event )
    {
        ff::write( bqt_out, "Touch event\n" );
    }
    // void handleTouchEvent( SDL_Event& sdl_event )
    // {
    //     bqt::window* active_window = getActiveWindow();
        
    //     switch( sdl_event.type )
    //     {
    //         // https://wiki.libsdl.org/SDL_TouchFingerEvent
    //         // http://hg.libsdl.org/SDL/file/default/README-gesture.txt
    //     case SDL_FINGERMOTION:
    //         break;
    //     case SDL_FINGERDOWN:
    //         break;
    //     case SDL_FINGERUP:
    //         break;
    //     case SDL_MULTIGESTURE:
    //         {
    //             if( sdl_event.mgesture.numFingers == 2 )                        // Ignore event.mgesture.numFingers != 2
    //             {
    //                 SDL_DisplayMode d_mode;
    //                 if( SDL_GetCurrentDisplayMode( SDL_GetWindowDisplayIndex( active_window -> getPlatformWindow().sdl_window ),
    //                                                &d_mode ) )                  // Current instead of Desktop for fullscreen
    //                 {
    //                     pinch_input p_i;
                        
    //                     p_i.distance = sdl_event.mgesture.dDist;
    //                     p_i.rotation = sdl_event.mgesture.dTheta;
    //                     p_i.position[ 0 ] = active_window -> getPosition().first - sdl_event.mgesture.x * d_mode.w;
    //                     p_i.position[ 1 ] = active_window -> getPosition().second - sdl_event.mgesture.y * d_mode.h;
    //                 }
    //                 else
    //                     throw exception( "handleTouchEvent(): Could not get display mode" );
    //             }
    //         }
    //         break;
    //     default:
    //         break;
    //     }
    // }
    
    void handleMotionEvent( XEvent& x_event )
    {
        bqt::window_event w_event;
        w_event.id = bqt::NONE;
        w_event.id = bqt::COMMAND;
        
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
                        w_event.id = bqt::STROKE;
                        
                        if( x_tablet_devices[ i ].type == ERASER_STYLUS )
                            w_event.stroke.click = CLICK_ERASE;
                        else
                            w_event.stroke.click = CLICK_PRIMARY;               // TODO: Figure out other click types from modifier keys
                        
                        // TODO: support less-than-fullscreen surfaces
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
                        
                        ff::write( bqt_out, w_event.stroke.position[ 0 ], " ", w_event.stroke.position[ 1 ], " ", w_event.stroke.pressure, "\n" );
                        break;
                    }
                }
            }
            break;
        }
        
        if( w_event.id != bqt::NONE )
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
        case VisibilityNotify:
        case MapNotify:
        case MapRequest:
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
    void openTabletDevices()
    {
        // for stroke_waypoint collecting:
        // http://www.cplusplus.com/reference/vector/vector/reserve/
        
        Display* x_display = getXDisplay();
        int x_screen = DefaultScreen( x_display );
        
        int x_tablet_name_count = sizeof( x_tablet_device_names )               // Array x_tablet_device_names
                                  / sizeof( x_tablet_device_name );             // Type x_tablet_device_name
        XEventClass x_eventclass_list[ x_tablet_name_count ];                   // We only need it as big as the most tablets we can have
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
                for( int j = 0; j < x_tablet_name_count; ++j )
                {
                    if( !strcmp( x_dev_info[ i ].name,
                                 x_tablet_device_names[ j ].name ) )
                    {
                        x_tablet_dev_detail detail;                             // I give up, these names are getting too long
                        
                        detail.name = x_tablet_device_names[ j ].name;
                        detail.type = x_tablet_device_names[ j ].type;
                        detail.x_devid = x_dev_info[ i ].id;
                        detail.x_device = XOpenDevice( x_display, detail.x_devid  );
                        
                        if( detail.x_device == NULL )
                            ff::write( bqt_out, "Warning: Found device \"", detail.name, "\" but could not open\n" );
                        else
                            if( getDevMode() )
                                ff::write( bqt_out, "Opened device \"", detail.name, "\"\n" );
                        
                        XAnyClassPtr x_any = ( XAnyClassPtr )x_dev_info[ i ].inputclassinfo;
                        for( int k = 0; k < x_dev_info[ i ].num_classes; ++k )
                        {
                            if( sizeof( XID ) != sizeof( long ) )               // TODO: Find a way to make this a compile-time error
                                throw exception( "openTabletDevices(): X class 'class' member hack does not work" );
                            // switch( x_any -> class )                         // OH NO YOU CAN'T DO THIS IS C++ BABY
                            switch( *( ( long* )x_any  ) )                      // 'class' should be sizeof( long ) & the first element of XAnyClass
                            {
                            case KeyClass:
                            case ButtonClass:
                                break;                                          // Ignore here
                            case ValuatorClass:
                                {
                                    XValuatorInfo* xval_info = ( XValuatorInfo* )x_any;
                                    
                                    if( xval_info -> mode != Absolute )
                                    {
                                        ff::write( bqt_out, "Warning: Tablet device found in relative mode, ignoring\n" );
                                        goto next_device;
                                    }
                                    
                                    if( xval_info -> num_axes != 6 )            // Throw exception until its supported (if necessary)
                                        throw exception( "openTabletDevices(): Tablet device with axis count != 6 found\n" );
                                    
                                    for( int axis = 0; axis < xval_info -> num_axes; ++axis )
                                        detail.axes[ axis ] = xval_info -> axes[ axis ];
                                }
                                break;
                            default:
                                ff::write( bqt_out, "unknown class\n" );
                                break;                                          // Ignore default
                            }
                            x_any = ( XAnyClassPtr )( ( char* )x_any + x_any -> length );   // THIS IS HOW YOU'RE SUPPOSED TO DO IT?
                        }
                        
                        DeviceMotionNotify( detail.x_device,
                                            detail.x_motioneventtype,
                                            x_eventclass_list[ x_eventclass_count ] );
                        ++x_eventclass_count;                                   // DeviceMotionNotify() is a macro, so increment outside
                        
                        if( 1000 / detail.axes[ 0 ].resolution                  // If the device l/mm is larger than the smallest screen l/mm
                            > ( x_screen_res[ 0 ] < x_screen_res[ 1 ] ? x_screen_res[ 0 ] : x_screen_res[ 1 ] ) )
                        {
                            ff::write( bqt_out, "Warning: Device \"", detail.name, "\" has a larger resolution than the screen\n" );
                        }
                        
                        x_tablet_devices.push_back( detail );
                        
                    next_device:
                        break;
                    }
                }
            }
            
            XSelectExtensionEvent( x_display,
                                   RootWindow( x_display, x_screen ),
                                   x_eventclass_list,
                                   x_eventclass_count );
        }
        XFreeDeviceList( x_dev_info );
        
        if( getDevMode() && x_tablet_devices.size() == 0 )
            ff::write( bqt_out, "No tablet devices found\n" );
    }
    
    void closeTabletDevices()
    {
        Display* x_display = getXDisplay();
        
        for( int i = 0; i < x_tablet_devices.size(); i++ )
            XCloseDevice( x_display, x_tablet_devices[ i ].x_device );
    }
    
    bool HandleEvents_task::execute( task_mask* caller_mask )
    {
        if( getQuitFlag() )
        {
            if( getDevMode() )
                ff::write( bqt_out, "Quitting...\n" );
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
                    closeTabletDevices();
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
                    break;                                                      // Ignore
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


