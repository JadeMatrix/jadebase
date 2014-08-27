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
        
        void wait( const mutex& wait_mutex ) const;                                   // Unlocks wait_mutex & pauses thread; on return relocks wait_mutex & unpauses thread
        // void wait_time( ... ) const;                                         // Timed wait, maybe implement later
        void signal() const;                                                    // Restart a single waiting thread
        void broadcast() const;                                                 // Restart all waiting threads
    };
}

/******************************************************************************//******************************************************************************/

#endif


