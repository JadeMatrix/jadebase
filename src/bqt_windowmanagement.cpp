/* 
 * bqt_windowmanagement.cpp
 * 
 * Implements the internal window management from bqt_windowmanagement.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_windowmanagement.hpp"

#include <map>

#include "bqt_mutex.hpp"
#include "bqt_exception.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_launchargs.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex wm_mutex;
    
    bqt::window* active_window = NULL;
    std::map< Uint32, bqt::window* > id_window_map;                             // SDL window id's are Uint32
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void registerWindow( window& w )
    {
        scoped_lock slock( wm_mutex );
        
        Uint32 window_id = SDL_GetWindowID( w.getPlatformWindow().sdl_window );
        
        if( id_window_map.count( window_id ) )
            throw exception( "registerWindow(): Window already registered" );
        else
            id_window_map[ window_id ] = &w;
        
        if( getDevMode() )
            ff::write( bqt_out, "Registered a window (", window_id, "), currently ", id_window_map.size(), " windows registered\n" );
    }
    void deregisterWindow( window& w )
    {
        scoped_lock slock( wm_mutex );
        
        Uint32 window_id = SDL_GetWindowID( w.getPlatformWindow().sdl_window );
        
        if( id_window_map.erase( window_id ) < 1 )
            throw exception( "deregisterWindow(): No window associated with platform window" );
        
        if( getDevMode() )
            ff::write( bqt_out, "Deregistered a window (", window_id, "), currently ", id_window_map.size(), " windows registered\n" );
    }
    
    bool isRegisteredWindow( bqt_platform_window_t& w )
    {
        scoped_lock slock( wm_mutex );
        
        return id_window_map.count( SDL_GetWindowID( w.sdl_window ) );
    }
    
    void makeWindowActive( bqt_platform_window_t& w )
    {
        scoped_lock slock( wm_mutex );
        
        Uint32 window_id = SDL_GetWindowID( w.sdl_window );
        
        if( id_window_map.count( window_id ) )
            active_window = id_window_map[ window_id ];
        else
            throw exception( "makeWindowActive(): No window associated with platform window" );
    }
    window* getActiveWindow()
    {
        scoped_lock slock( wm_mutex );
        
        return active_window;
    }
    window& getWindow( bqt_platform_window_t& w )
    {
        scoped_lock slock( wm_mutex );
        
        Uint32 window_id = SDL_GetWindowID( w.sdl_window );
        
        if( id_window_map.count( window_id ) )
            return *( id_window_map[ window_id ] );
        else
        {
            ff::write( bqt_out, "No window associated with id ", window_id, "\n" );
            throw exception( "getWindow(): No window associated with platform window" );
        }
    }
    
    void closeAllWindows()
    {
        scoped_lock slock( wm_mutex );
        
        window::manipulate* wmanip = NULL;
        task_mask close_mask = TASK_SYSTEM;
        
        for( int i = id_window_map.size(); i > 0; --i )
        {
            wmanip = new window::manipulate( id_window_map.begin() -> second );
            wmanip -> close();
            
            if( wmanip -> execute( &close_mask ) )                              // This is a dirty hack but it was the nicest of the current alternatives
                delete wmanip;
            else
                throw exception( "closeAllWindows(): Failed to close a window" );
        }
        
        id_window_map.clear();                                                  // Just in case, but shouldn't be needed
    }
}


