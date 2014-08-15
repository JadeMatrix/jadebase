#ifndef BQT_RWLOCK_HPP
#define BQT_RWLOCK_HPP

/* 
 * bqt_rwlock.hpp
 * 
 * Contains bqt::rwlock, a class for manipulating & managing read-write locks.
 * bqt::rwlocks are 'shared' by default.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../bqt_platform.h"
#include "bqt_threadutil.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class rwlock
    {
    protected:
        bqt_platform_rwlock_t platform_rwlock;
    public:
        rwlock();
        ~rwlock();
        
        void lock_read() const;
        bool try_read() const;
        
        void lock_write() const;
        bool try_write() const;
        
        void unlock() const;
    };
    
    #define RW_READ  false
    #define RW_WRITE true
    
    class scoped_lock_RWLOCK                                                    // TODO: Replace with scoped_lock<> from bqt_scopedlock.hpp
    {
    private:
        rwlock& slrwl;
    public:
        scoped_lock_RWLOCK( rwlock& r, bool m = RW_READ ) : slrwl( r )
        {
            if( m )
                slrwl.lock_write();
            else
                slrwl.lock_read();
        }
        ~scoped_lock_RWLOCK()
        {
            slrwl.unlock();
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


