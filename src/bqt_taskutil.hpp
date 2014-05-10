#ifndef BQT_TASKUTIL_HPP
#define BQT_TASKUTIL_HPP

/* 
 * bqt_taskutil.hpp
 * 
 * Utility file for the task system.  Defines task flags/masks.
 * 
 * Task masks were born out of necessity.  Ideally any task could execute on any
 * thread.  However, some system-specific APIs are not thread-safe and in fact
 * MUST be executed on the main thread.  OpenGL calls are also not thread-safe;
 * this could be worked around by mutexing for call batches, but that essential-
 * ly acheives the same result as GPU tasks, just with more mess and confusion.
 * 
 * When used by a task, the mask describes where it is allowed to execute.  When
 * used by a thread, the mask describes the restrictions on what it can execute.
 * Thus, if !( ~thread mask & task mask ) then execute (however this can be
 * overridden).
 * 
 * A task should only ever identify as one code/mask.  On the other hand, a task
 * worker thread can have multiple codes ORed into a mask; this mask can change
 * at any point during the thread's lifetime.
 * 
 */

/******************************************************************************//******************************************************************************/

namespace bqt
{
    typedef unsigned char task_mask;                                            // Describes where the task is allowed to execute
    
    static task_mask TASK_ALL    = 0x00; /* 00000000 */                         // Thread: "All possible restrictions" Task: "Any thread"
    static task_mask TASK_NONE   = 0xFF; /* 11111111 */                         // Thread: "No restrictions" Task: "No thread"
    
    static task_mask TASK_GPU    = 0x01; /* 00000001 */                         // Thread: "Allow GPU tasks" Task: "Restrict to GPU threads"
    static task_mask TASK_INPUT  = 0x02; /* 00000010 */                         // ... same for input tasks
    static task_mask TASK_SYSTEM = 0x04; /* 00000100 */                         // System initialization thread (usually main thread)
    static task_mask TASK_TASK   = 0x08; /* 00001000 */                         // Task system specifically
    static task_mask TASK_VIDEO  = 0x10; /* 00010000 */                         // Video system, if any, seperate from GPU
    
    typedef unsigned char task_priority;
    
    static task_priority PRIORITY_LOW  = 0x00;
    static task_priority PRIORITY_NONE = 0x01;
    static task_priority PRIORITY_HIGH = 0x02;
}

/******************************************************************************//******************************************************************************/

#endif


