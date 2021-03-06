#ifndef JADEBASE_EVENTS_HPP
#define JADEBASE_EVENTS_HPP

/* 
 * jb_events.hpp
 * 
 * System events are handled and window_events are distributed from a single
 * task, which loops by being requeued (returning false from execute()).
 * 
 * jadebase makes reasonable effort to support the various features of disparate
 * hardware input devices in some kind of unified way in the light of this
 * software's functionality, and as supported by the platforms it runs on. Any
 * such support/conversions will make as few assumptions as possible; any such
 * assumptions should be documented.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../tasking/jb_task.hpp"
#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class HandleEvents_task : public task
    {
    public:
        bool execute( task_mask* );
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


