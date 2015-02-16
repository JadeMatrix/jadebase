#ifndef JADEBASE_CONDITION_HPP
#define JADEBASE_CONDITION_HPP

/* 
 * jb_condition.hpp
 * 
 * Contains jade::condition, a class definition for manipulating & managing
 * conditions as objects.  Conditions need private access to mutices to access
 * the internal platform data, so jade::condition is marked as a friend class in
 * jade::mutex.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_mutex.hpp"
#include "../threading/jb_threadutil.hpp"
#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class condition
    {
    protected:
        jb_platform_condition_t platform_condition;
    public:
        condition();
        ~condition();
        
        void wait( const mutex& wait_mutex ) const;                             // Unlocks wait_mutex & pauses thread; on return relocks wait_mutex & unpauses
                                                                                // thread
        // void wait_time( ... ) const;                                         // Timed wait, maybe implement later
        void signal() const;                                                    // Restart a single waiting thread
        void broadcast() const;                                                 // Restart all waiting threads
    };
}

/******************************************************************************//******************************************************************************/

#endif


