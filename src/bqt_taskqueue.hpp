#ifndef BQT_TASKQUEUE_HPP
#define BQT_TASKQUEUE_HPP

/* 
 * bqt_taskqueue.hpp
 * 
 * Contains bqt::task_queue, a class used for storing & sorting tasks waiting to
 * be executed.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <list>

#include "bqt_taskutil.hpp"
#include "bqt_task.hpp"
#include "bqt_mutex.hpp"
#include "bqt_condition.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
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
        task* pop( task_mask mask );                                            // See bqt_taskutil.h; returns NULL if not open
        
        void push( task* item );
        
        void open();
        void close();                                                           // Broadcasts to tq_cond so that the queue can be deleted safely; also deletes
                                                                                // any unexecuted tasks
        
        int size();                                                             // For debugging & benchmarking
    };
}

/******************************************************************************//******************************************************************************/

#endif


