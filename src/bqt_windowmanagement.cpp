/* 
 * bqt_windowmanagement.cpp
 * 
 * Implements the internal window management from bqt_windowmanagement.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_windowmanagement.hpp"

#include <map>

#include "threading/bqt_mutex.hpp"
#include "bqt_exception.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_launchargs.hpp"

#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex wm_mutex;
    
    // bqt::window* active_window = NULL;
    std::map< Window, bqt::window* > id_window_map;                             // SDL window id's are Uint32
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void registerWindow( window& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        Window window_id = w.getPlatformWindow().x_window;
        
        if( id_window_map.count( window_id ) )
            throw exception( "registerWindow(): Window already registered" );
        else
            id_window_map[ window_id ] = &w;
        
        if( getDevMode() )
            ff::write( bqt_out,
                       "Registered a window (id 0x",
                        ff::to_x( ( unsigned long )window_id ),
                        "), currently ",
                        id_window_map.size(),
                        " windows registered\n" );
    }
    void deregisterWindow( window& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        Window window_id = w.getPlatformWindow().x_window;
        
        bqt::window* erased_window = id_window_map[ window_id ];
        
        if( id_window_map.erase( window_id ) < 1 )
            throw exception( "deregisterWindow(): No window associated with platform window" );
        
        if( getDevMode() )
            ff::write( bqt_out,
                       "Deregistered a window (id 0x",
                        ff::to_x( ( unsigned long )window_id ),
                        "), currently ",
                        id_window_map.size(),
                        " windows registered\n" );
    }
    
    bool isRegisteredWindow( bqt_platform_window_t& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        return id_window_map.count( w.x_window );
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
    window& getWindow( bqt_platform_window_t& w )
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        Window window_id = w.x_window;
        
        if( id_window_map.count( window_id ) )
            return *( id_window_map[ window_id ] );
        else
        {
            exception e;
            ff::write( *e, "getWindow(): No window associated with platform window id ", ff::to_x( window_id ) );
            throw e;
        }
    }
    
    void closeAllWindows()
    {
        scoped_lock< mutex > slock( wm_mutex );
        
        window::manipulate* wmanip = NULL;
        task_mask close_mask = TASK_SYSTEM;
        
        for( int i = id_window_map.size(); i > 0; --i )
        {
            wmanip = new window::manipulate( id_window_map.begin() -> second );
            wmanip -> close();
            
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
        
        for( std::map< Window, bqt::window* >::iterator iter = id_window_map.begin();
             iter != id_window_map.end();
             ++iter )
        {
            iter -> second -> requestRedraw();
        }
    }
}


