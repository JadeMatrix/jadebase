/* 
 * bqt_taskexec.cpp
 * 
 * Implements bqt_taskexec.hpp; stores the internal state & data for the task
 * system.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_taskexec.hpp"

#include "bqt_platform.h"
#if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
#include <time.h>
#else
// TODO: Implement timing on other platforms
#error "Timing not implemented on non-POSIX platforms"
#endif

#include "bqt_taskqueue.hpp"
#include "bqt_exception.hpp"
#include "threading/bqt_threadutil.hpp"
#include "threading/bqt_mutex.hpp"
#include "threading/bqt_thread.hpp"
#include "bqt_log.hpp"
#include "bqt_launchargs.hpp"
#include "threading/bqt_condition.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

#define MIN_SLEEP_TIME   100000
#define MAX_SLEEP_TIME  2000000

namespace
{
    struct task_thread_data
    {
        bqt::task_queue* queue;
        bqt::task_mask* mask;
    };
    
    bqt::task_queue* global_task_queue;                                         // This doesn't seem to work unless it's dynamically allocated
    bqt::thread* task_threads = NULL;
    task_thread_data* task_threads_data = NULL;
    bqt::task_mask* task_threads_masks = NULL;
    long spawned_task_thread_count = 0;
    
    bqt::mutex task_thread_count_mutex;
    long task_thread_count = 0;
    
    bool arrested = false;
    bool arrest_continue = true;
    bqt::mutex arrest_mutex;
    bqt::condition arrest_condition;
    bool arrestHook()
    {
        bqt::scoped_lock< bqt::mutex > slock( arrest_mutex );
        
        if( arrested )
            arrest_condition.wait( arrest_mutex );
        
        return arrest_continue;
    }
    
    bqt::exit_code taskThread( void* d )                                        // A cee_taskexec::task_thread_data* is passed as a void*
    {
        task_thread_data* data = ( task_thread_data* )d;
        bqt::exit_code code = EXIT_FINE;
        
        #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
        timespec rest_time;
        rest_time.tv_sec = 0;                                                   // We're never going to rest for a full second
        #else
        #error "Timing not implemented on non-POSIX platforms"
        #endif
        
        int queue_size;
        
        if( bqt::isInitTaskSystem() )
        {
            {
                bqt::scoped_lock< bqt::mutex > slock( task_thread_count_mutex );
                task_thread_count++;
            }
            
            try
            {
                bool running = true;
                bqt::task* current_task;
                
                while( running )
                {
                    if( arrestHook() )
                    {
                        if( data -> mask == NULL )
                            current_task = data -> queue -> pop();
                        else
                            current_task = data -> queue -> pop( *( data -> mask ) );
                        
                        if( current_task != NULL )                              // current_task will be NULL if the task system is ending
                        {
                            if( current_task -> execute( data -> mask ) )       // Try executing the tast, re-push if requeue requested
                                delete current_task;
                            else
                                data -> queue -> push( current_task );
                            
                            #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
                            queue_size = data -> queue -> size();               // Dynamic waiting between tasks - more in queue, faster
                            if( queue_size > 0 )
                                rest_time.tv_nsec = MIN_SLEEP_TIME + ( MAX_SLEEP_TIME - MIN_SLEEP_TIME ) / queue_size;
                            else
                                rest_time.tv_nsec = MAX_SLEEP_TIME;
                            
                            nanosleep( &rest_time, NULL );                      // Only sleep if we're continuing
                            #else
                            #error "Timing not implemented on non-POSIX platforms"
                            #endif
                        }
                        else
                            running = false;                                    // Popped a null task so exit
                    }
                    else
                    {
                        running = false;
                        code = bqt::EXITCODE_ARREST;
                    }
                }
            }
            catch( bqt::exception& e )
            {
                ff::write( bqt_out, "BQTDraw exception from task thread: ", e.what(), "\n" );
                
                code = bqt::EXITCODE_BQTERR;
            }
            catch( std::exception& e )
            {
                ff::write( bqt_out, "Exception from task thread: ", e.what(), "\n" );
                
                code = bqt::EXITCODE_STDERR;
            }
            
            {
                bqt::scoped_lock< bqt::mutex > slock( task_thread_count_mutex );
                task_thread_count--;
            }
            
            return code;
        }
        else
            throw bqt::exception( "taskThread(): Task system not initialized" );
    }
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool initTaskSystem( bool main_is_tasker )
    {
        long thread_limit = getTaskThreadLimit();
        long core_count = getSystemCoreCount();
        long thread_count;
        
        if( ( thread_limit > 0 ) && ( thread_limit < core_count ) )
            thread_count = thread_limit;
        else
            thread_count = core_count;
        
        if( main_is_tasker )
            return initTaskSystem( thread_count - 1 );
        else
            return initTaskSystem( thread_count );
    }
    bool initTaskSystem( long tc )
    {
        if( tc < 0 )
            return false;
        
        long thread_limit = getTaskThreadLimit();
        if( thread_limit > 1 && tc > thread_limit )
            tc = thread_limit;
        
        global_task_queue  = new task_queue();
        spawned_task_thread_count = tc;
        task_threads       = new           thread[ tc ];
        task_threads_data  = new task_thread_data[ tc ];
        task_threads_masks = new        task_mask[ tc ];
        
        for( int i = 0; i < tc; ++i )
        {
            task_threads_masks[i] = TASK_ALL;
            
            task_threads_data[i].queue = global_task_queue;
            task_threads_data[i].mask  = &( task_threads_masks[i] );
            
            task_threads[i].setFunction( &taskThread );
            task_threads[i].setData( ( void* )( &( task_threads_data[i] ) ) );
            task_threads[i].start();
        }
        
        global_task_queue -> open();
        
        if( getDevMode() )
            ff::write( bqt_out, "Spawned ", tc, " task threads\n" );
        
        return true;
    }
    bool isInitTaskSystem()
    {
        // Ugh, not very nice
        
        if( task_threads == NULL || task_threads_data == NULL )
        {
            if( task_threads == NULL && task_threads_data == NULL )
                return false;
            else
                throw exception( "isinitTaskSystem(): Task system only partially inited" );
        }
        else
            return true;
    }

    void stopTaskSystem()
    {
        global_task_queue -> close();                                           // Task queue will now pop NULLs
        
        for( int i = 0; i < spawned_task_thread_count; ++i )
        {
            exit_code ec = task_threads[i].wait();
            
            if( getDevMode() )
                ff::write( bqt_out, "Worker thread exited with code ", exc2str( ec ), "\n" );
        }
    }
    
    void arrestTaskSystem()
    {
        scoped_lock< mutex > slock( arrest_mutex );
        
        arrested = true;
        arrest_continue = true;
    }
    void releaseTaskSystem()
    {
        scoped_lock< mutex > slock( arrest_mutex );
        
        arrested = false;
        
        arrest_condition.broadcast();
    }
    
    void deInitTaskSystem()
    {
        delete[] task_threads;
        delete[] task_threads_data;
        
        task_threads = NULL;
        task_threads_data = NULL;
    }

    // STOPTASKSYSTEM_TASK /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool StopTaskSystem_task::execute( task_mask* caller_mask )
    {
        if( ( *caller_mask ) & TASK_TASK )                                      // A little sanity check
            stopTaskSystem();
        else
            throw exception( "StopTaskSystem_task::execute(): Calling thread's mask does not contain TASK_TASK" );
        
        return true;
    }
    task_mask StopTaskSystem_task::getMask()
    {
        return TASK_TASK;
    }

    // OTHER STUFF /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void submitTask( task* t )
    {
        global_task_queue -> push( t );
    }

    exit_code becomeTaskThread( task_mask* mask )
    {
        if( isInitTaskSystem() )
        {
            task_thread_data data;
            data.queue = global_task_queue;
            data.mask = mask;
            
            return taskThread( ( void* )( &data ) );
        }
        else
            throw exception( "becomeTaskThread(): Task system not initialized" );
    }

    int getTaskQueueSize()
    {
        if( isInitTaskSystem() )
            return global_task_queue -> size();
        else
            return -1;
    }
    long getTaskThreadCount()
    {
        scoped_lock< mutex > slock( task_thread_count_mutex );
        return task_thread_count;
    }
}


