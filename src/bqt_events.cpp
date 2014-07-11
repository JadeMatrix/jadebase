/* 
 * bqt_events.cpp
 * 
 * About
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
#include "bqt_platform.h"
#include "bqt_exception.hpp"
#include "bqt_launchargs.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    std::map< Uint32, bqt::window::manipulate* > window_manipulates;            // Waiting room so we don't submit more than we need
    
    // INPUT DEVICES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // http://www.wacomeng.com/mac/Developers%20Guide.htm
    
    typedef unsigned int click_type;
    #define CLICK_PRIMARY   0x0001      // 0000 0001
    #define CLICK_SECONDARY 0x0002      // 0000 0010
    #define CLICK_ALT       0x0004      // 0000 0100
    #define CLICK_ERASE     0x0008      // 0000 1000
    #define CLICK_LENS      0x0010      // 0001 0000
    
    struct substroke
    {
        click_type click;
        int   start_pos[ 2 ];                                                   // Position [ x, y ] relative to screen
        int     end_pos[ 2 ];
        int   start_pres;                                                       // Pressure
        int     end_pres;
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
    
    // EVENT HANDLERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void handleWindowEvent( SDL_Event& sdl_event )
    {
        using namespace bqt;
        
        bqt_platform_window_t platform_window;
        platform_window.sdl_window = SDL_GetWindowFromID( sdl_event.window.windowID );
        
        if( isRegisteredWindow( platform_window ) )
        {
            if( !window_manipulates.count( sdl_event.window.windowID ) )        // Create a new manipulate task if necessary
                window_manipulates[ sdl_event.window.windowID ] = new window::manipulate( &getWindow( platform_window ) );
        }
        else
        {
            if( getDevMode() )
                ff::write( bqt_out, "SDL window event received for unregistered window, ignoring\n" );
            
            return;
        }
        
        window::manipulate* current_manip = window_manipulates[ sdl_event.window.windowID ];
        
        switch( sdl_event.window.event )
        {
            case SDL_WINDOWEVENT_SHOWN:
                current_manip -> redraw();
                break;
            case SDL_WINDOWEVENT_HIDDEN:
                
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                current_manip -> redraw();
                break;
            case SDL_WINDOWEVENT_MOVED:
                // Don't need to actually change position (in fact breaks)
                // current_manip -> setPosition( sdl_event.window.data1, sdl_event.window.data2 );
                break;
            case SDL_WINDOWEVENT_RESIZED:
                current_manip -> setDimensions( sdl_event.window.data1, sdl_event.window.data2 );
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                // broken on Ubuntu 14.04?
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                current_manip -> minimize();
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                current_manip -> maximize();
                break;
            case SDL_WINDOWEVENT_RESTORED:
                current_manip -> restore();
                break;
            case SDL_WINDOWEVENT_ENTER:
                
                break;
            case SDL_WINDOWEVENT_LEAVE:
                
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                current_manip -> setFocus( true );
                makeWindowActive( platform_window );
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                current_manip -> setFocus( false );
                break;
            case SDL_WINDOWEVENT_CLOSE:
                current_manip -> close();
                break;
        }
    }
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void eventsSetUp()
    {
        // http://www.cplusplus.com/reference/vector/vector/reserve/
        
        
    }
    
    bool HandleEvents_task::execute( task_mask* caller_mask )
    {
        {
            SDL_Event sdl_event;
            
            while( SDL_PollEvent( &sdl_event ) )                                // Ugh, unsafe: not guaranteed to terminate
            {
                switch( sdl_event.type )
                {
                case SDL_DOLLARGESTURE:
                    break;
                case SDL_DROPFILE:
                    break;
                case SDL_FINGERMOTION:
                    break;
                case SDL_FINGERDOWN:
                    break;
                case SDL_FINGERUP:
                    break;
                case SDL_KEYDOWN:
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_JOYAXISMOTION:
                    break;
                case SDL_JOYBALLMOTION:
                    break;
                case SDL_JOYHATMOTION:
                    break;
                case SDL_JOYBUTTONDOWN:
                    break;
                case SDL_JOYBUTTONUP:
                    break;
                case SDL_MOUSEMOTION:
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    break;
                case SDL_MOUSEBUTTONUP:
                    break;
                case SDL_MOUSEWHEEL:
                    break;
                case SDL_MULTIGESTURE:
                    break;
                case SDL_QUIT:
                    {
                        if( getDevMode() )
                            ff::write( bqt_out, "Quitting...\n" );
                        closeAllWindows();
                        submitTask( new StopTaskSystem_task() );
                    }
                    break;
                case SDL_SYSWMEVENT:
                    break;
                case SDL_TEXTEDITING:
                    break;
                case SDL_TEXTINPUT:
                    break;
                case SDL_USEREVENT:
                    break;
                case SDL_WINDOWEVENT:
                    handleWindowEvent( sdl_event );
                    break;
                default:
                    NULL;
                }
            }
            
            for( std::map< Uint32, bqt::window::manipulate* >::iterator iter = window_manipulates.begin();
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


