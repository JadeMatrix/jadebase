#ifndef JADEBASE_THREAD_HPP
#define JADEBASE_THREAD_HPP

/* 
 * jb_thread.hpp
 * 
 * Contains jade::thread, a class definition for manipulating & managing threads
 * as objects.
 * 
 */

/* TODO: In light of the move to C++11, the threading facilities should be
 * most gutted (for example using std::mutex instead of jade::mutex).  However,
 * std::thread is pretty gimped and not ideal, so a proposal - a 'thread'
 * variadic template class as such:
 * 
 *      template< typename Return, typename... Args > class thread
 *      {
 *      public:
 *          thread( Return (*)( Args... ) );
 *          ...
 *          void start( Args... );
 *          Return wait();
 *      protected:
 *          // Probably a std::thread*, as std::thread starts on creation
 *      };
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_threadutil.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class thread
    {
    public:
        thread();                                                               // Create a thread with no function and no parameter data
        thread( thread_func );                                                  // Create a thread with the given function but with no parameter data
        thread( void* );                                                        // Create a thread with no function but with the given parameter data
        thread( thread_func, void* );                                           // Greate a thread with the given function and parameter data
        ~thread();                                                              // Cancels any currently running thread; please try to exit normally first
        
        void setFunction( thread_func );                                        // Sets the thread function, replacing any existing function
        void setData( void* );                                                  // Sets the thread parameter data, replacing any existing data
        
        bool start();                                                           // Starts the thread using the existing function and parameter data
        bool start( thread_func );                                              // Starts the thread using the given function but the existing parameter data
        bool start( void* );                                                    // Starts the thread using the existing function but the given parameter data
        bool start( thread_func, void* );                                       // Starts the thread using the given function and parameter data
        
        exit_code wait();                                                       // Waits for the thread to finish execution and joins with it, returning the
                                                                                // thread's return code.
        
    protected:
        jb_platform_thread_t platform_thread;
        bool has_thread;
        thread_func function;
        void* data;
        
        void createPlatformThread( thread_func, void* );
        
        void kill();
    };
}

/******************************************************************************//******************************************************************************/

#endif


