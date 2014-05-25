/* 
 * bqt_trackable.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_trackable.hpp"

#include <stack>

#include "bqt_mutex.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct history_event
    {
        bqt::timestamp stamp;
        bqt::trackable* object;                                                      // TODO: change to reference if possible
    };
    
    bqt::mutex history_mutex;
    std::stack< history_event > undo_history;
    std::stack< history_event > redo_history;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    /* TRACKABLE **************************************************************//******************************************************************************/
    
    void trackable::registerChange( timestamp stamp )
    {
        scoped_lock slock( history_mutex );
        
        history_event he = { stamp, this };
        
        undo_history.push( he );
    }
    
    /**************************************************************************//******************************************************************************/
    
    int undoLastChange()
    {
        scoped_lock slock( history_mutex );
        
        if( undo_history.empty() )
            return HIST_NONELEF;
        else
        {
            history_event swap = undo_history.top();
            undo_history.pop();
            
            int result = swap.object -> undo( swap.stamp );
            
            if( !result )
                redo_history.push( swap );
            else
                return result;                                                  // Discard event
        }
        
        return HIST_SUCCESS;
    }
    int redoLastChange()
    {
        scoped_lock slock( history_mutex );
        
        if( redo_history.empty() )
            return HIST_NONELEF;
        else
        {
            history_event swap = redo_history.top();
            redo_history.pop();
            
            int result = swap.object -> redo( swap.stamp );
            
            if( !result )
                undo_history.push( swap );
            else
                return result;                                                  // Discard event
        }
        
        return HIST_SUCCESS;
    }
}


