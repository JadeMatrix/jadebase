/* 
 * bqt_thread.cpp
 * 
 * Implements the bqt::thread class from bqt_thread.hpp using pthreads
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_thread.hpp"

#include "../bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    
    void thread::createPlatformThread( thread_func function, void* data )
    {
        if( has_thread )
            throw exception( "thread::createPlatformThread(): Thread already running" );
        if( function == NULL )
            throw exception( "thread::createPlatformThread(): function is NULL" );
        
        int err;
        
        if( ( err = pthread_attr_init( &platform_thread.pt_attr ) ) )
            throw exception( "thread::createPlatformThread(): Could not create attributes: " + errc2str( err ) );
        if( ( err = pthread_attr_setdetachstate( &platform_thread.pt_attr,
                                                 PTHREAD_CREATE_JOINABLE ) ) )  // Create a 'joinable' attribute for the thread
            throw exception( "thread::createPlatformThread(): Could not set detach state: " + errc2str( err ) );
        
        if( ( err = pthread_create( &platform_thread.pt_thread, &platform_thread.pt_attr, function, data ) ) )
        {
            has_thread = false;                                                 // Keep state correct in case exception is caught
            throw exception( "thread::createPlatformThread(): Could not create thread: " + errc2str( err ) );
        }
        else
            has_thread = true;
    }
    
    void thread::kill()
    {
        if( has_thread )
        {
            int err;
            
            if( ( err = pthread_cancel( platform_thread.pt_thread ) ) )
                throw exception( "thread::kill(): Failed to cancel thread: " + errc2str( err ) );
        }
    }
    
    thread::thread()
    {
        has_thread = false;
        
        function = NULL;
        data = NULL;
    }
    thread::thread( thread_func function )
    {
        has_thread = false;
        
        this -> function = function;
        data = NULL;
    }
    thread::thread( void* data )
    {
        has_thread = false;
        
        function = NULL;
        this -> data = data;
    }
    thread::thread( thread_func function, void* data )
    {
        has_thread = false;
        
        this -> function = function;
        this -> data = data;
    }
    thread::~thread()
    {
        kill();
    }
    
    void thread::setFunction( thread_func function )
    {
        this -> function = function;
    }
    void thread::setData( void* data )
    {
        this -> data = data;
    }
    
    bool thread::start()
    {
        createPlatformThread( function, data );
        
        return has_thread;
    }
    bool thread::start( thread_func function )
    {
        createPlatformThread( function, data );
        
        return has_thread;
    }
    bool thread::start( void* data )
    {
        createPlatformThread( function, data );
        
        return has_thread;
    }
    bool thread::start( thread_func function, void* data )
    {
        createPlatformThread( function, data );
        
        return has_thread;
    }
    
    exit_code thread::wait()
    {
        exit_code return_value;
        
        int err;
        
        if( ( err = pthread_join( platform_thread.pt_thread, &return_value ) ) )
            throw exception( "thread::wait(): Failed to join thread: " + errc2str( err ) );
        
        has_thread = false;
        
        return return_value;
    }
    
    #else
    
    #error "Threads not implemented on non-POSIX platforms"
    
    #endif
}


