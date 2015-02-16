#ifndef JADEBASE_THREAD_HPP
#define JADEBASE_THREAD_HPP

/* 
 * jb_thread.hpp
 * 
 * Contains jade::thread, a class definition for manipulating & managing threads
 * as objects.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_threadutil.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class thread
    {
    protected:
        jb_platform_thread_t platform_thread;
        bool has_thread;
        thread_func function;
        void* data;
        
        void createPlatformThread( thread_func function, void* data );
        
        void kill();
    public:
        thread();
        thread( thread_func function );
        thread( void* data );
        thread( thread_func function, void* data );
        ~thread();                                                              // Cancels any currently running thread; please try to exit normally first
        
        void setFunction( thread_func function );
        void setData( void* data );
        
        bool start();
        // These do not change the stored function & data pointers
        bool start( thread_func function );
        bool start( void* data );
        bool start( thread_func function, void* data );
        
        exit_code wait();   
    };
}

/******************************************************************************//******************************************************************************/

#endif


