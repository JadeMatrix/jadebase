/* 
 * jb_trackable.cpp
 * 
 * Implements the implementable stuff in jb_trackable.hpp
 * 
 */

// TODO: Per-document change tracking

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_trackable.hpp"

#include <deque>
#include <map>

#include "threading/jb_mutex.hpp"
#include "jb_exception.hpp"
#include "jb_settings.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct history_event
    {
        jade::timestamp stamp;
        jade::trackable* object;                                                      // TODO: change to reference if possible
    };
    
    jade::mutex history_mutex;
    std::deque< jade::timestamp > undo_stamps;
    std::multimap< jade::timestamp, jade::trackable* > undo_history;
    std::deque< jade::timestamp > redo_stamps;
    std::multimap< jade::timestamp, jade::trackable* > redo_history;
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* TRACKABLE **************************************************************//******************************************************************************/
    
    void trackable::registerChange( timestamp stamp )
    {
        scoped_lock< mutex > slock( history_mutex );
        
        undo_stamps.push_back( stamp );
        undo_history.insert( std::pair< jade::timestamp, jade::trackable* >( stamp, this ) );
        
        redo_stamps.clear();
        redo_history.clear();
        
        long undo_steps = getSetting_num( "jb_MaxUndo" );
        if( undo_steps >= 0 && undo_stamps.size() > undo_steps )
        {
            if( undo_stamps.size() > ( undo_steps + 1 ) )                       // Sanity check, should always be false (we remove whenever one goes out)
                throw exception( "trackable::registerChange(): Multiple stamps hanging out of window" );
            
            undo_history.erase( *( undo_stamps.begin() ) );                     // Remove all trackable pointers associated with the oldest timestamp 
            undo_stamps.pop_front();                                            // Remove the oldest timestamp
        }
    }
    
    /**************************************************************************//******************************************************************************/
    
    int undoLastChange()
    {
        scoped_lock< mutex > slock( history_mutex );
        
        if( undo_stamps.empty() )
            return HIST_NONELEF;
        else
        {
            timestamp last_change = undo_stamps.back();
            undo_stamps.pop_back();
            
            std::pair< std::multimap< jade::timestamp,
                                      jade::trackable* >::iterator,
                       std::multimap< jade::timestamp,
                                      jade::trackable* >::iterator > iters = undo_history.equal_range( last_change );
            
            std::multimap< jade::timestamp,
                           jade::trackable* >::iterator r_first = iters.first;
            std::multimap< jade::timestamp,
                           jade::trackable* >::iterator r_last = iters.second;
            
            int ret_val = HIST_SUCCESS;
            
            for( ; r_first != r_last; ++r_first )
            {
                int result = r_first -> second -> undo( last_change );
                
                if( result != HIST_SUCCESS )
                {
                    if( result == HIST_NOSTAMP )
                        throw exception( "undoLastChange(): trackable has no change matching stamp" );
                    
                    else ret_val = result;
                }                                                               // Drop event
                else
                {
                    redo_stamps.push_back( last_change );
                    redo_history.insert( std::pair< jade::timestamp, jade::trackable* >( last_change, r_first -> second ) );
                }                                                               // Keep event
            }
            
            undo_history.erase( last_change );
            
            return ret_val;
        }
    }
    int redoLastChange()
    {
        scoped_lock< mutex > slock( history_mutex );
        
        if( redo_stamps.empty() )
            return HIST_NONELEF;
        else
        {
            timestamp prev_change = redo_stamps.back();
            redo_stamps.pop_back();
            
            std::pair< std::multimap< jade::timestamp,
                                      jade::trackable* >::iterator,
                       std::multimap< jade::timestamp,
                                      jade::trackable* >::iterator > iters = redo_history.equal_range( prev_change );
            
            std::multimap< jade::timestamp,
                           jade::trackable* >::iterator r_first = iters.first;
            std::multimap< jade::timestamp,
                           jade::trackable* >::iterator r_last = iters.second;
            
            int ret_val = HIST_SUCCESS;
            
            for( ; r_first != r_last; ++r_first )
            {
                int result = r_first -> second -> redo( prev_change );
                
                if( result != HIST_SUCCESS )
                {
                    if( result == HIST_NOSTAMP )
                        throw exception( "redoLastChange(): trackable has no change matching stamp" );
                    
                    else ret_val = result;
                }                                                               // Drop event
                else
                {
                    undo_stamps.push_back( prev_change );
                    undo_history.insert( std::pair< jade::timestamp, jade::trackable* >( prev_change, r_first -> second ) );
                }                                                               // Keep event
            }
            
            redo_history.erase( prev_change );
            
            return ret_val;
        }
    }
}


