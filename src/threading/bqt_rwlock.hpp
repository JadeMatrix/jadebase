#ifndef BQT_RWLOCK_HPP
#define BQT_RWLOCK_HPP

/* 
 * bqt_rwlock.hpp
 * 
 * Contains bqt::rwlock, a class for manipulating & managing read-write locks.
 * bqt::rwlocks are 'shared' by default.
 * 
 * There is a slight difference between the lock_*() and try_*() methods:
 *   - The lock_*() methods lock the rwlock if it is not already locked by the
 *     calling thread, then retun true.  If the calling thread already controls
 *     the lock, they return false.  The caller must eventually unlock if they
 *     return true, and may not unlock if they return false. In addition, if a
 *     single thread attempts a write lock more than once, it blocks waiting on
 *     itself.
 *   - The try_*() methods use a non-blocking test on the lock.  If they suc-
 *     ceed, the rwlock is locked, they return true, and the caller must even-
 *     tually unlock.  Otherwise they return false, the caller may not unlock,
 *     but is free to continue execution.
 * Due to the complexity of the lock functions, utilizing scoped_lock< rwlock >
 * is ALWAYS recommended.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../bqt_platform.h"
#include "bqt_threadutil.hpp"
#include "bqt_scopedlock.hpp"
#include "bqt_condition.hpp"
#include "bqt_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class rwlock
    {
    protected:
        condition rwlock_cond;
        mutex     rwlock_mutex;
        unsigned long writer_id;
        int access_count;
    public:
        rwlock();
        
        void lock_read() const;
        bool try_read() const;                                                  // Returns true on success, false on failure
        
        void lock_write() const;
        bool try_write() const;                                                 // Returns true on success, false on failure
        
        void unlock() const;
    };
    
    /* SCOPED_LOCK SPECIALIZATION *********************************************//******************************************************************************/
    
    #define RW_READ  false
    #define RW_WRITE true
    
    template<> class scoped_lock< rwlock >
    {
    private:
        const rwlock& slrwl;
    public:
        scoped_lock( const rwlock& r, bool m ) : slrwl( r )
        {
            if( m )
                slrwl.lock_write();
            else
                slrwl.lock_read();
        }
        ~scoped_lock()
        {
            slrwl.unlock();
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


