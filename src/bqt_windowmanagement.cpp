/* 
 * bqt_windowmanagement.cpp
 * 
 * Implements the internal window management from bqt_windowmanagement.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_windowmanagement.hpp"

#include <map>

#include "threading/bqt_rwlock.hpp"
#include "bqt_exception.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_launchargs.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::rwlock wm_lock;
    
    bqt::window* active_window = NULL;
    std::map< Window, bqt::window* > id_window_map;                             // SDL window id's are Uint32
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void registerWindow( window& w )
    {
        scoped_lock< rwlock > slock( wm_lock, RW_WRITE );
        
        // Uint32 window_id = SDL_GetWindowID( w.getPlatformWindow().x_window );
        Window window_id = w.getPlatformWindow().x_window;
        
        if( id_window_map.count( window_id ) )
            throw exception( "registerWindow(): Window already registered" );
        else
            id_window_map[ window_id ] = &w;
        
        if( getDevMode() )
            ff::write( bqt_out, "Registered a window (id 0x", ff::to_x( ( unsigned long )window_id ), "), currently ", id_window_map.size(), " windows registered\n" );
    }
    void deregisterWindow( window& w )
    {
        scoped_lock< rwlock > slock( wm_lock, RW_WRITE );
        
        // Uint32 window_id = SDL_GetWindowID( w.getPlatformWindow().sdl_window );
        Window window_id = w.getPlatformWindow().x_window;
        
        bqt::window* erased_window = id_window_map[ window_id ];
        
        if( id_window_map.erase( window_id ) < 1 )
            throw exception( "deregisterWindow(): No window associated with platform window" );
        
        if( erased_window == active_window )
        {
            if( id_window_map.size() )
            {
                active_window = id_window_map.begin() -> second;
                
                window::manipulate* active_manip = new window::manipulate( active_window );
                active_manip -> makeActive();
                
                submitTask( active_manip );
            }
            else
                active_window = NULL;
        }
        
        if( getDevMode() )
            ff::write( bqt_out, "Deregistered a window (id 0x", ff::to_x( ( unsigned long )window_id ), "), currently ", id_window_map.size(), " windows registered\n" );
    }
    
    bool isRegisteredWindow( bqt_platform_window_t& w )
    {
        scoped_lock< rwlock > slock( wm_lock );
        
        // return id_window_map.count( SDL_GetWindowID( w.sdl_window ) );
        return id_window_map.count( w.x_window );
    }
    
    int getRegisteredWindowCount()
    {
        scoped_lock< rwlock > slock( wm_lock );
        
        return id_window_map.size();
    }
    
    void makeWindowActive( bqt_platform_window_t& w )
    {
        scoped_lock< rwlock > slock( wm_lock, RW_WRITE );
        
        // Uint32 window_id = SDL_GetWindowID( w.sdl_window );
        Window window_id = w.x_window;
        
        if( id_window_map.count( window_id ) )
            active_window = id_window_map[ window_id ];
        else
            throw exception( "makeWindowActive(): No window associated with platform window" );
    }
    window* getActiveWindow()
    {
        scoped_lock< rwlock > slock( wm_lock );
        
        return active_window;
    }
    window& getWindow( bqt_platform_window_t& w )
    {
        scoped_lock< rwlock > slock( wm_lock );
        
        // Uint32 window_id = SDL_GetWindowID( w.sdl_window );
        Window window_id = w.x_window;
        
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
        scoped_lock< rwlock > slock( wm_lock, RW_WRITE );
        
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


