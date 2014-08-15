/* 
 * bqt_mutex.cpp
 * 
 * Implements bqt::mutex from bqt_mutex.hpp; bqt::scoped_lock is trivial and is
 * implemented in bqt_mutex.hpp.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_mutex.hpp"

#include "../bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    
    mutex::mutex()
    {
        int err;
        
        if( ( err = pthread_mutexattr_init( &platform_mutex.pt_attr ) ) )
            throw exception( "mutex::mutex(): Could not initialize mutex attributes: " + errc2str( err ) );
        
        if( ( err = pthread_mutexattr_settype( &platform_mutex.pt_attr, PTHREAD_MUTEX_RECURSIVE ) ) )
            throw exception( "mutex::mutex(): Could not set mutex recursive: " + errc2str( err ) );
        
        if( ( err = pthread_mutex_init( &platform_mutex.pt_mutex, &platform_mutex.pt_attr ) ) )
            throw exception( "mutex::mutex(): Could not create mutex: " + errc2str( err ) );
    }
    mutex::~mutex()
    {
        int err;
        
        if( ( err = pthread_mutex_destroy( &platform_mutex.pt_mutex ) ) )
            throw exception( "mutex::~mutex(): Could not destroy mutex: " + errc2str( err ) );
        
        if( ( err = pthread_mutexattr_destroy( &platform_mutex.pt_attr ) ) )
            throw exception( "mutex::~mutex(): Could not destroy mutex attributes");
    }
    void mutex::lock() const
    {
        int err;
        
        if( ( err = pthread_mutex_lock( const_cast< pthread_mutex_t* >( &platform_mutex.pt_mutex ) ) ) )
            throw exception( "mutex::lock(): Failed to lock mutex: " + errc2str( err ) );
    }
    void mutex::unlock() const
    {
        int err;
        
        if( ( err = pthread_mutex_unlock( const_cast< pthread_mutex_t* >( &platform_mutex.pt_mutex ) ) ) )
            throw exception( "mutex::unlock(): Failed to unlock mutex: " + errc2str( err ) );
    }
    bool mutex::try_lock() const
    {
        return pthread_mutex_trylock( const_cast< pthread_mutex_t* >( &platform_mutex.pt_mutex ) );
    }
    
    #else
    
    #error "Muteces not implemented on non-POSIX platforms"
    
    #endif
}


