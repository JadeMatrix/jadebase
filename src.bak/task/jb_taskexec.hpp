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
#include "threading/jb_threadutil.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    bool initTaskSystem( bool main_is_tasker = false );
    bool initTaskSystem( long tc );                                             // Returns false if tc < 0 or already running
    bool isInitTaskSystem();                                                    // Very rough right now, usage unadvised
    
    void stopTaskSystem();
    
    void arrestTaskSystem();
    void releaseTaskSystem();
    
    void deInitTaskSystem();                                                    // Must only be called AFTER stopTaskSystem(), which allows self-stopping
    
    class StopTaskSystem_task : public task
    {
    public:
        bool execute( task_mask* caller_mask );
        task_priority getPriority()
        {
            return PRIORITY_HIGH;
        }
        task_mask getMask();
    };
    
    void submitTask( task* t );                                                 // Of course this is not guaranteed safe if system isn't inited
                                                                                // Once a task is submitted, the task system takes full control of the pointer
                                                                                // (ie it handles deletion of the data)
    
    exit_code becomeTaskThread( task_mask* mask );                              // Turns the calling thread into a task solving thread
                                                                                // Meant for use by main() but should work with any thread
    
    // For debugging & benchmarking
    int getTaskQueueSize();                                                     // Returns -1 on error
    long getTaskThreadCount();
}

/******************************************************************************//******************************************************************************/

#endif


