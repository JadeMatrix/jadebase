#error bqt_scopedlock.hpp is a placeholder and not intended for inclusion at this time

#ifndef BQT_SCOPEDLOCK_HPP
#define BQT_SCOPEDLOCK_HPP

/* 
 * bqt_scopedlock.hpp
 * 
 * Template class for scope-locking various thread safety mechanisms
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_mutex.hpp"
#include "bqt_semaphore.hpp"
#include "bqt_rwlock.hpp"
// #include "bqt_spinlock.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    template< class LOCKABLE > class scoped_lock
    {
    };
    
    /* SPECIALIZATIONS ********************************************************//******************************************************************************/
    
    template<> class scoped_lock< mutex >
    {
    private:
        mutex& slm;
    public:
        scoped_lock( mutex& m ) : slm( m )
        {
            slm.lock();
        }
        ~scoped_lock()
        {
            slm.unlock();
        }
    };
    
    template<> class scoped_lock< semaphore >
    {
    private:
        semaphore& sls;
        unsigned int count;
    public:
        scoped_lock( semaphore& m ) : sls( m ), count( 0 )
        {
            sls.acquireAll();
        }
        scoped_lock( semaphore& m, unsigned int c ) : sls( m ), count( c )
        {
            sls.acquire( count );
        }
        ~scoped_lock()
        {
            if( count )
                sls.release( count );
            else
                sls.releaseAll();
        }
    };
    
    #define RW_READ  false
    #define RW_WRITE true
    
    template<> class scoped_lock< rwlock >
    {
    private:
        rwlock& slrwl;
    public:
        scoped_lock( rwlock& r, bool m = RW_READ ) : slrwl( r )
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


