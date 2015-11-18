#ifndef JADEBASE_TASKEXEC_HPP
#define JADEBASE_TASKEXEC_HPP

/* 
 * jb_taskexec.hpp
 * 
 * Contains the infrastructure for running & operating the task system
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "jb_taskutil.hpp"
#include "jb_task.hpp"
#include "../threading/jb_threadutil.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    bool initTaskSystem( bool main_is_tasker = false );                         // Initialize with a single thread per core (minus one if main_is_tasker = true)
    bool initTaskSystem( long );                                                // Initialize with a specific number of task threads; returns false if tc < 0 or
                                                                                // already running
    bool isInitTaskSystem();                                                    // Very rough right now, usage unadvised
    
    void stopTaskSystem();
    
    void arrestTaskSystem();
    void releaseTaskSystem();
    
    void deInitTaskSystem();                                                    // Must only be called AFTER stopTaskSystem(), which allows self-stopping
    
    // TODO: Remove
    class StopTaskSystem_task : public task
    {
    public:
        bool execute( task_mask* );
        task_priority getPriority()
        {
            return PRIORITY_HIGH;
        }
        task_mask getMask();
    };
    
    void submitTask( task* );                                                   // Of course this is not guaranteed safe if system isn't initialized.
                                                                                // Once a task is submitted, the task system takes full control of the pointer
                                                                                // (ie it handles deletion of the data).
    
    exit_code becomeTaskThread( task_mask* );                                   // Turns the calling thread into a task solving thread, passing a pointer to the
                                                                                // mask to be used; meant for use by main() but should work with any thread.
    
    // For debugging & benchmarking
    int getTaskQueueSize();                                                     // Returns -1 on error
    long getTaskThreadCount();
}

/******************************************************************************//******************************************************************************/

#endif


