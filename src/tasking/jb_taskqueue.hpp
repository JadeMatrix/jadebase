#ifndef JADEBASE_TASKQUEUE_HPP
#define JADEBASE_TASKQUEUE_HPP

/* 
 * jb_taskqueue.hpp
 * 
 * Contains jade::task_queue, a class used for storing & sorting tasks waiting
 * to be executed.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <list>

#include "jb_task.hpp"
#include "jb_taskutil.hpp"
#include "../threading/jb_condition.hpp"
#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class task_queue
    {
    private:
    protected:
        std::list< task* > data[3];                                             // This is kind of a sanity hack, there's probably a more efficient way
        mutex tq_mutex;
        condition tq_cond;
        enum
        {
            PREOPEN,
            OPEN,
            CLOSED
        } status;
    public:
        task_queue();
        // ~task_queue();
        
        task* pop();                                                            // Calls pop( TASK_NONE )
        task* pop( task_mask mask );                                            // See jb_taskutil.h; returns NULL if not open
        
        void push( task* item );
        
        void open();
        void close();                                                           // Broadcasts to tq_cond so that the queue can be deleted safely; also deletes
                                                                                // any unexecuted tasks
        
        int size();                                                             // For debugging & benchmarking
    };
}

/******************************************************************************//******************************************************************************/

#endif


