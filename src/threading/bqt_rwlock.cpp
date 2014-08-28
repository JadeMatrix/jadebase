/* 
 * bqt_rwlock.cpp
 * 
 * Implements bqt_rwlock.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_rwlock.hpp"

#include <errno.h>

#include "../bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    
    rwlock::rwlock()
    {
        int err;
        
        if( ( err = pthread_rwlockattr_init( &platform_rwlock.pt_attr ) ) )
            throw exception( "rwlock::rwlock(): Could not initialize rwlock attributes: " + errc2str( err ) );
        
        if( ( err = pthread_rwlockattr_setpshared( &platform_rwlock.pt_attr, PTHREAD_PROCESS_SHARED ) ) )
            throw exception( "rwlock::rwlock(): Could not make rwlock shared: " + errc2str( err ) );
        
        if( ( err = pthread_rwlock_init( &platform_rwlock.pt_rwlock, &platform_rwlock.pt_attr ) ) )
            throw exception( "rwlock::rwlock(): Could not create rwlock: " + errc2str( err ) );
    }
    rwlock::~rwlock()
    {
        int err;
        
        writer.pt_thread = 0x00;
        
        if( ( err = pthread_rwlock_destroy( &platform_rwlock.pt_rwlock ) ) )
            throw exception( "rwlock::~rwlock(): Could not destroy rwlock: " + errc2str( err ) );
        
        if( ( err = pthread_rwlockattr_destroy( &platform_rwlock.pt_attr ) ) )
            throw exception( "rwlock::~rwlock(): Could not destroy rwlock attributes: " + errc2str( err ) );
    }
    
    bool rwlock::lock_read() const
    {
        int err;
        
        if( ( err = pthread_rwlock_rdlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) ) ) )
        {
            if( err != EDEADLK )                                                // This is OK, it just means this thread already has a lock
                throw exception( "rwlock::lock_write(): Could not get a read lock: " + errc2str( err ) );
            
            return false;
        }
        
        return true;
    }
    bool rwlock::try_read() const
    {
        return !pthread_rwlock_tryrdlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) );
    }
    
    bool rwlock::lock_write() const
    {
        scoped_lock< mutex > slock( writer_mutex );
        
        while( pthread_rwlock_trywrlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) ) )
        {
            if( writer.pt_thread == pthread_self() )
                return false;
            
            writer_cond.wait( writer_mutex );
        }
        
        *const_cast< pthread_t* >( &writer.pt_thread ) = pthread_self();
        
        // This apparently does not work on Linux even though the POSIX spec says it should, try on other platforms:
        // int err;
        // if( ( err = pthread_rwlock_wrlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) ) ) )
        // {
        //     if( err != EDEADLK )                                                // This is OK, it just means this thread already has a lock
        //         throw exception( "rwlock::lock_write(): Could not get a write lock: " + errc2str( err ) );
            
        //     return false;
        // }
        
        return true;
    }
    bool rwlock::try_write() const
    {
        return !pthread_rwlock_trywrlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) );
    }
    
    void rwlock::unlock() const
    {
        int err;
        
        if( ( err = pthread_rwlock_unlock( const_cast< pthread_rwlock_t* >( &platform_rwlock.pt_rwlock ) ) ) )
            throw exception( "rwlock::unlock(): Could not unlock: " + errc2str( err ) );
        
        writer_cond.signal();
    }
    
    #else
    
    #error "Muteces not implemented on non-POSIX platforms"
    
    #endif
}


