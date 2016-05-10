/* 
 * jb_windowmanagement.cpp
 * 
 * Implements the internal window management from jb_windowmanagement.hpp
 * 
 */

// TODO: Platform guards

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_windowmanagement.hpp"

#include <map>

#include "../tasking/jb_taskexec.hpp"
#include "../threading/jb_mutex.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex wm_mutex;
    
    std::map< jb_platform_window_t,
              jade::window*,
              jade::jb_platform_window_t_less_t > id_window_map( jade::jb_platform_window_t_less );
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    void registerWindow( window& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        jb_platform_window_t platform_window = w.getPlatformWindow();
        
        if( id_window_map.count( platform_window ) )
            throw exception( "registerWindow(): Window already registered" );
        else
            id_window_map[ platform_window ] = &w;
        
        if( getDevMode() )
            // ff::write( jb_out,
            //            "Registered a window (id 0x",
            //             ff::to_x( ( unsigned long )window_id,
            //                       PTR_HEX_WIDTH,
            //                       PTR_HEX_WIDTH ),
            //             "), currently ",
            //             id_window_map.size(),
            //             " windows registered\n" );
            ff::write( jb_out,
                       "Registered a window, currently ",
                        id_window_map.size(),
                        " windows registered\n" );
    }
    void deregisterWindow( window& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        jb_platform_window_t platform_window = w.getPlatformWindow();
        
        if( id_window_map.erase( platform_window ) < 1 )
            throw exception( "deregisterWindow(): No window associated with platform window" );
        
        if( getDevMode() )
            // ff::write( jb_out,
            //            "Deregistered a window (id 0x",
            //             ff::to_x( ( unsigned long )window_id,
            //                       PTR_HEX_WIDTH,
            //                       PTR_HEX_WIDTH ),
            //             "), currently ",
            //             id_window_map.size(),
            //             " windows registered\n" );
            ff::write( jb_out,
                       "Deregistered a window, currently ",
                        id_window_map.size(),
                        " windows registered\n" );
        
        if( id_window_map.size() < 1 )
        {
            bool should_quit;
            if( !getSetting( "jb_QuitOnNoWindows", should_quit ) )
                #ifdef PLATFORM_MACOSX
                should_quit = false;
                #else
                should_quit = true;
                #endif
            
            if( should_quit )
            {
                if( getDevMode() )
                    ff::write( jb_out, "All windows closed, quitting\n" );
                
                jb_setQuitFlag();
            }
        }
    }
    
    bool isRegisteredWindow( jb_platform_window_t& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        return id_window_map.count( w );
    }
    
    int getRegisteredWindowCount()
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        return id_window_map.size();
    }
    
    window& getAnyWindow()
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        if( !id_window_map.size() )
            throw exception( "getAnyWindow(): No windows registered" );
        else
            return *( id_window_map.begin() -> second );                        // Just grab the 'first' iterable one
    }
    window& getWindow( jb_platform_window_t& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        if( id_window_map.count( w ) )
            return *( id_window_map[ w ] );
        else
        {
            exception e;
            ff::write( *e,
                       "getWindow(): No window associated with platform window" );
            throw e;
        }
    }
    
    void closeAllWindows()
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        // TODO: Perchance we can just loop over the windows and call their
        // close()?  Need to test if this was a concurrency issue; note that
        // manually executing a ManipulateWindow_task may lead to concurrency
        // errors if closing windows must happen on the main thread, as there
        // is nothing prevening closeAllWindows() from being called elsewhere.
        #if 0
        for( int i = id_window_map.size(); i > 0; --i )
            id_window_map.begin() -> second -> close();
        #endif
        
        window::ManipulateWindow_task* wmanip = NULL;
        task_mask close_mask = TASK_SYSTEM;
        
        for( int i = id_window_map.size(); i > 0; --i )
        {
            wmanip = new window::ManipulateWindow_task( id_window_map.begin() -> second );
            wmanip -> updates.close = true;
            
            if( wmanip -> execute( &close_mask ) )                              // Executing the manipulate task with the close flag removes the window from the
                                                                                // the list, which is why we don't iterate over the list.  This is a dirty hack
                                                                                // but it was the nicest of the current alternatives
                delete wmanip;
            else
                throw exception( "closeAllWindows(): Failed to close a window" );
        }
        
        id_window_map.clear();                                                  // Just in case, but shouldn't be needed
    }
    
    void redrawAllWindows()
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        for( auto iter = id_window_map.begin();
             iter != id_window_map.end();
             ++iter )
        {
            iter -> second -> requestRedraw();
        }
    }
}


