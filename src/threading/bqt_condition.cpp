/* 
 * bqt_condition.cpp
 * 
 * Implements bqt::condition from bqt_condition.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_condition.hpp"

#include "../bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // May need this for timed waits eventually if they are implemented
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    
    condition::condition()
    {
        int err;
        
        if( ( err = pthread_cond_init( &platform_condition.pt_cond, NULL ) ) )
            throw exception( "condition::condition(): Could not create condition: " + errc2str( err ) );
    }
    condition::~condition()
    {
        int err;
        
        if( ( err = pthread_cond_destroy( &platform_condition.pt_cond ) ) )
            throw exception( "condition::~condition(): Could not destroy condition: " + errc2str( err ) );
    }

    void condition::wait( mutex& wait_mutex )
    {
        int err;
        
        if( ( err = pthread_cond_wait( &platform_condition.pt_cond, &( wait_mutex.platform_mutex.pt_mutex ) ) ) )
            throw exception( "condition::wait(): Condition wait failed: " + errc2str( err ) );
    }
    // void wait_time( ... )
    // {
    //     //
    // }
    void condition::signal()
    {
        int err;
        
        if( ( err = pthread_cond_signal( &platform_condition.pt_cond ) ) )
            throw exception( "condition::signal(): Condition signal failed: " + errc2str( err ) );
    }
    void condition::broadcast()
    {
        int err;
        
        if( ( err = pthread_cond_broadcast( &platform_condition.pt_cond ) ) )
            throw exception( "condition::broadcast(): Condition broadcast failed: " + errc2str( err ) );
    }
    
    #else
    
    #error "Conditions not implemented on non-POSIX platforms"
    
    #endif
}

