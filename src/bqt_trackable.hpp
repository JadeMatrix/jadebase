#ifndef BQT_TRACKABLE_HPP
#define BQT_TRACKABLE_HPP

/* 
 * bqt_trackable.hpp
 * 
 * Contains a pure virtual class bqt::trackable used as an interface for change
 * history.  Classes that inherit from bqt::trackable are responsible for
 * keeping track of their own history, implementing undo/redo, and ensuring
 * thread safety.
 * bqt::trackable::registerChange() will alert the history system that a change
 * took place on this object at the given time.  Each timestamp is considered
 * a unique event; if more than one change is registered for a given timestamp
 * (even if the objects are different) they will be undone/redone at the same
 * time.
 * undo() and redo() step backward & forward through changes.  undo(stamp) and
 * redo(stamp) take the timestamp of a change, revert from/to that state, then
 * return 0x00 (HIST_SUCCESS) or an error code:
 * HIST_NONELEF     No more undo/redo steps
 * HIST_NOSTAMP     No step associated with timestamp
 * HIST_FAILURE     Any other kind of failure
 * undo()/redo() should submit any appropriate update tasks.  registerChange()
 * should be called by any appropriate task (friended of course).
 * 
 * undoLastChange() and redoLastChange() work like undo() and redo() but on a
 * global scale, working on the global bqt::trackable stack.  They undo/redo
 * ALL changes associated with the most recent timestamp in the respective
 * stack.
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_timestamp.hpp"

/******************************************************************************//******************************************************************************/

#define HIST_SUCCESS    0x00
#define HIST_NONELEF    0x01
#define HIST_NOSTAMP    0x02
#define HIST_FAILURE    0x03

namespace bqt
{
    class trackable
    {
    protected:
        void registerChange( timestamp stamp );
    public:
        virtual int undo() = 0;
        virtual int redo() = 0;
        virtual int undo( timestamp stamp ) = 0;
        virtual int redo( timestamp stamp ) = 0;
    };
    
    int undoLastChange();
    int redoLastChange();
}

/******************************************************************************//******************************************************************************/

#endif


