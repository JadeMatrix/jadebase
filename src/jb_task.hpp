#ifndef JADEBASE_TASK_HPP
#define JADEBASE_TASK_HPP

/* 
 * jb_task.hpp
 * 
 * Contains the pure virtual class jade::task to serve as a base/interface for
 * creating custom task classes.  jade::task uses C++'s virtual inheritance for
 * this execute() overloading as the performance hit should be minimal.
 * 
 * jade::task contains a virtual function matchMask( mask ) that should be used
 * for mask matching rather than getMask() to allow the default algorithm to be
 * overridden by children.  getMask() still exists so that tasks can opt whether
 * to store the mask as a variable or simply return it to save space.  It is
 * pure virtual to encourage an explicit decision.
 * 
 * To submit tasks from non-C++ code (such as Objective-C for Cocoa), it unfor-
 * tunately seems necessary to write a C wrapper function that should follow
 * the format:
 *      void submit_TaskName_task( ... )
 *      {
 *          jade::submitTask( new TaskName_task( ... ) );
 *      }
 * with the usual #ifdef __cplusplus/extern "C" guards, of course.  Arguments
 * may be passed to the function in a C-friendly form and be converted as
 * needed.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_taskutil.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class task
    {
    public:
        virtual ~task() {};                                                     // Safety
        
        // TODO: pass a task_mask& instead of task_mask*
        virtual bool execute( task_mask* caller_mask ) = 0;                     // Returns true on success, false to re-queue
        
        virtual task_priority getPriority()
        {
            return PRIORITY_NONE;
        }
        
        virtual task_mask getMask() = 0;
        
        virtual bool matchMask( task_mask m )                                   // Supplies the default matching algorithm
        {
            return !( ~m & getMask() );                                         // See jb_taskutil.h
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


