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
 *     return true, and may not unlock if they return false.
 *   - The try_*() methods use a non-blocking test on the lock.  If they suc-
 *     ceed, the rwlock is locked, they return true, and the caller must even-
 *     tually unlock.  Otherwise they return false, the caller may not unlock,
 *     but is free to continue execution.
 * Due to the complexity of the lock functions, utilizing scoped_lock< rwlock >
 * is ALWAYS recommended; in addition, the try_* methods may be removed in the
 * future.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../bqt_platform.h"
#include "bqt_threadutil.hpp"
#include "bqt_scopedlock.hpp"

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
        
        bool lock_read() const;                                                 // Returns true if it needs unlocking when done (ie first time locked by thread)
        bool try_read() const;                                                  // Returns true on success, false on failure
        
        bool lock_write() const;                                                // Returns true if it needs unlocking when done (ie first time locked by thread)
        bool try_write() const;                                                 // Returns true on success, false on failure
        
        void unlock() const;
    };
    
    /* SCOPED_LOCK SPECIALIZATION *********************************************//******************************************************************************/
    
    #define RW_READ  false
    #define RW_WRITE true
    
    template<> class scoped_lock< rwlock >
    {
    private:
        rwlock& slrwl;
        bool unlock;
    public:
        scoped_lock( rwlock& r, bool m = RW_READ ) : slrwl( r )
        {
            if( m )
                unlock = slrwl.lock_write();
            else
                unlock = slrwl.lock_read();
            
            // unlock = m ? slrwl.lock_write() : slrwl.lock_read();
        }
        ~scoped_lock()
        {
            if( unlock )
                slrwl.unlock();
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


