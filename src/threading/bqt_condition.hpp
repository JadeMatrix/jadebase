#ifndef BQT_CONDITION_HPP
#define BQT_CONDITION_HPP

/* 
 * bqt_condition.hpp
 * 
 * Contains bqt::condition, a class definition for manipulating & managing
 * conditions as objects.  Conditions need private access to mutices to access
 * the internal platform data, so bqt::condition is marked as a friend class in
 * bqt::mutex.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../bqt_platform.h"
#include "bqt_threadutil.hpp"
#include "bqt_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class condition
    {
    protected:
        bqt_platform_condition_t platform_condition;
    public:
        condition();
        ~condition();
        
        void wait( mutex& wait_mutex );                                         // Unlocks wait_mutex & pauses thread; on return relocks wait_mutex & unpauses thread
        // void wait_time( ... )                                                // Timed wait, maybe implement later
        void signal();                                                          // Restart a single waiting thread
        void broadcast();                                                       // Restart all waiting threads
    };
}

/******************************************************************************//******************************************************************************/

#endif


