/* 
 * bqt_rwlock.cpp
 * 
 * Implements bqt_rwlock.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_rwlock.hpp"

#include <errno.h>

#include "../bqt_platform.h"
#include "../bqt_exception.hpp"
#include "../bqt_log.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    
    rwlock::rwlock()
    {
        writer_id = 0x00;
        access_count = 0;
    }
    
    void rwlock::lock_read() const
    {
        scoped_lock< mutex > slock( rwlock_mutex );
        
        while( writer_id != 0x00 && writer_id != pthread_self() )
            rwlock_cond.wait( rwlock_mutex );
        
        ++( *const_cast< int* >( &access_count ) );                             // Haha what
    }
    bool rwlock::try_read() const
    {
        scoped_lock< mutex > slock( rwlock_mutex );
        
        if( writer_id != 0x00 && writer_id != pthread_self() )
            return false;
        else
        {
            ++( *const_cast< int* >( &access_count ) );
            return true;
        }
    }
    
    void rwlock::lock_write() const
    {
        scoped_lock< mutex > slock( rwlock_mutex );
        
        while( writer_id != pthread_self() && writer_id != 0x00 && access_count )                                          // Wait for all current read & write locks
            rwlock_cond.wait( rwlock_mutex );
        
        *const_cast< unsigned long* >( &writer_id ) = pthread_self();
        ++( *const_cast< int* >( &access_count ) );
    }
    bool rwlock::try_write() const
    {
        scoped_lock< mutex > slock( rwlock_mutex );
        
        if( writer_id != pthread_self() && writer_id != 0x00 && access_count )
        {
            return false;
        }
        else
        {
            *const_cast< unsigned long* >( &writer_id ) = pthread_self();
            ++( *const_cast< int* >( &access_count ) );
            return true;
        }
    }
    
    void rwlock::unlock() const
    {
        scoped_lock< mutex > slock( rwlock_mutex );
        
        --( *const_cast< int* >( &access_count ) );
        
        if( access_count < 0 )
            throw exception( "rwlock::unlock(): Access count < 0" );
        
        if( !access_count )
            *const_cast< unsigned long* >( &writer_id ) = 0x00;
        
        rwlock_cond.signal();
    }
    
    #else
    
    #error "Muteces not implemented on non-POSIX platforms"
    
    #endif
}


