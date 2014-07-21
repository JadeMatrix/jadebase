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
#include "bqt_platform.h"
#include "bqt_exception.hpp"
#include "bqt_launchargs.hpp"
#include "bqt_mutex.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

#ifdef PLATFORM_XWS_GNUPOSIX

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    std::map< Window, bqt::window::manipulate* > window_manipulates;            // Waiting room so we don't submit more than we need
    
    // INPUT DEVICES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct
    {
        const char* dev_name;
        bool is_eraser;
    } x_tablet_device_names[] = { { "wacomdev1"                                   , false },
                                  { "wacomdev2"                                   , true  },
                                  { "Wacom Serial Penabled 2FG Touchscreen stylus", false },
                                  { "Wacom Serial Penabled 2FG Touchscreen eraser", true  },
                                  { "Wacom Serial Penabled 2FG Touchscreen touch" , false } };
                                  
    #endif
    
    // http://www.wacomeng.com/mac/Developers%20Guide.htm
    
    typedef unsigned short click_type;
    #define CLICK_PRIMARY   0x0001      // 0000 0001
    #define CLICK_SECONDARY 0x0002      // 0000 0010
    #define CLICK_ALT       0x0004      // 0000 0100
    #define CLICK_ERASE     0x0008      // 0000 1000
    #define CLICK_LENS      0x0010      // 0001 0000
    
    struct substroke
    {
        click_type click;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;                                                         // Apple Option/Alt or Windows Alt
        bool meta  : 1;                                                         // Apple Command or Windows key
        
        float start_pos[ 2 ];                                                   // Position [ x, y ] relative to screen (fractional if supported)
        float   end_pos[ 2 ];
        float start_pres;                                                       // Pressure
        float   end_pres;
        int   start_tilt[ 2 ];                                                  // Tilt [ x, y ]
        int     end_tilt[ 2 ];
        float start_rot;                                                        // Rotation (0.0 up to but not including 1.0 is a full rotation, can contain
        float   end_rot;                                                        // multiple rotations)
        float start_tang;                                                       // Tangential pressure -1.0 through 1.0
        float   end_tang;
    };
    
    struct linear_input
    {
        std::vector< substroke > substrokes;                                    // All substrokes in the current stroke
        int prev_subsroke_set_end;                                              // Position of last substroke from previous event handling
        
        linear_input()
        {
            prev_subsroke_set_end = -1;
        }
    };
    
    std::map< bqt_platform_idevid_t, linear_input > linear_inputs;
    
    struct key_input
    {
        bqt_platform_keycode_t key;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;
        bool meta  : 1;
    };
    
    struct pinch_input
    {
        float distance;                                                         // Relative change in distance
        float rotation;                                                         // Relative change, 0.0 through 1.0 for a full rotation, repeating
        int position[ 2 ];                                                      // Absolute position in-window
    };
    
    // X QUIT HANDLING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef PLATFORM_XWS_GNUPOSIX
    
    bqt::mutex quit_mutex;
    bool quit_flag = false;
    
    #endif
    
    // EVENT HANDLERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    void handleKeyEvent( XEvent& x_event )
    {
        
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
    
    // void handleMouseEvent( SDL_Event& sdl_event )
    // {
    //     // https://wiki.libsdl.org/SDL_MouseButtonEvent#Remarks
    //     // ignore SDL_TOUCH_MOUSEID
    //     switch( sdl_event.type )
    //     {
    //     case SDL_MOUSEMOTION:
    //         break;
    //     case SDL_MOUSEBUTTONDOWN:
    //         break;
    //     case SDL_MOUSEBUTTONUP:
    //         break;
    //     case SDL_MOUSEWHEEL:
    //         break;
    //     default:
    //         break;
    //     }
    // }
    
    void handleTextEvent( XEvent& x_event )
    {
        // http://www.x.org/releases/X11R7.6/doc/libX11/specs/XIM/xim.html
    }
    // void handleTextEvent( SDL_Event& sdl_event )
    // {
    //     switch( sdl_event.type )
    //     {
    //     case SDL_TEXTEDITING:
    //         break;
    //     case SDL_TEXTINPUT:
    //         break;
    //     default:
    //         break;
    //     }
    // }
    
    void handleWindowEvent( XEvent& x_event )
    {
        using namespace bqt;
        
        bqt_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        
        if( isRegisteredWindow( platform_window ) )
        {
            if( !window_manipulates.count( platform_window.x_window ) )
                window_manipulates[ platform_window.x_window ] = new window::manipulate( &getWindow( platform_window ) );
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
            {
                XConfigureRequestEvent& x_cfg_event( x_event.xconfigurerequest );
                
                if( x_cfg_event.value_mask & CWX || x_cfg_event.value_mask & CWY )
                    current_manip -> setPosition( x_cfg_event.x, x_cfg_event.y );
                
                if( x_cfg_event.value_mask & CWWidth || x_cfg_event.value_mask & CWHeight )
                {
                    if( x_cfg_event.width < 1 || x_cfg_event.height < 1 )       // Trust no one
                        throw exception( "handleWindowEvent(): Width or height not within limits" );
                    
                    current_manip -> setDimensions( x_cfg_event.width, x_cfg_event.height );
                }
            }
            break;
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
}

/******************************************************************************//******************************************************************************/

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
    void eventsSetUp()
    {
        // http://www.cplusplus.com/reference/vector/vector/reserve/
        
        
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
            
            for( int queue_size = XEventsQueued( x_display, QueuedAfterFlush ); // AKA XPending( x_display )
                 queue_size > 0;
                 --queue_size )                                                 // Yay we can guarantee termination
            {
                XNextEvent( x_display, &x_event );
                
                switch( x_event.type )
                {
                    case Expose:
                    case ConfigureRequest:
                    case MapRequest:
                    case ClientMessage:
                        handleWindowEvent( x_event );
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


