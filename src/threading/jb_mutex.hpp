#ifndef JADEBASE_MUTEX_HPP
#define JADEBASE_MUTEX_HPP

/* 
 * jb_mutex.hpp
 * 
 * Contains jade::mutex, a class definition for manipulating & managing mutices
 * as objects.  Conditions need private access to mutices to access the internal
 * platform data, so jade::condition is marked as a friend class in jade::mutex.
 * Also contains jade::scoped_lock, a superior way to use mutices compared to
 * lock()/unlock() blocks.  It is safe to wait a condition on a scope-locked
 * mutex.
 * 
 * Note: jade::mutex allows recursive locks.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_scopedlock.hpp"
#include "jb_threadutil.hpp"
#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class mutex
    {
        friend class condition;
        
    public:
        mutex();
        ~mutex();
        
        // These need to be const to allow thread-safety as well as const-safety in data types
        void lock() const;
        void unlock() const;
        bool try_lock() const;                                                  // Returns true if lock was successful, false if it was not.
        
    protected:
        jb_platform_mutex_t platform_mutex;
    };
}

/******************************************************************************//******************************************************************************/

#endif


