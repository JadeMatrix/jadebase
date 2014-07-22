#ifndef BQT_EVENTS_HPP
#define BQT_EVENTS_HPP

/* 
 * bqt_events.hpp
 * 
 * System events are handled and window_events are distributed from a single
 * task, which loops by being requeued (returning false from execute()).
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_task.hpp"
#include "bqt_log.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class HandleEvents_task : public task
    {
    public:
        bool execute( task_mask* caller_mask );
        task_mask getMask()
        {
            return TASK_SYSTEM;
        }
        // task_priority getPriority()
        // {
        //     return PRIORITY_HIGH;
        // }
    };
}

/******************************************************************************//******************************************************************************/

#endif


