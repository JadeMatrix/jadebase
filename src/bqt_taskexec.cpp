/* 
 * bqt_taskexec.cpp
 * 
 * Implements bqt_taskexec.hpp; stores the internal state & data for the task
 * system.
 * 
 * Things that still need to be ported from the CEE implementation:
 *  - thread arresting
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_taskexec.hpp"

#include <time.h>

#include "bqt_taskqueue.hpp"
#include "bqt_exception.hpp"
#include "bqt_threadutil.hpp"
#include "bqt_mutex.hpp"
#include "bqt_thread.hpp"
#include "bqt_log.hpp"
#include "bqt_launchargs.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

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
    
    bqt::exit_code taskThread( void* d )                                        // A cee_taskexec::task_thread_data* is passed as a void*
    {
        task_thread_data* data = ( task_thread_data* )d;
        bqt::exit_code code = EXIT_FINE;
        
        timespec rest_time;                                                     // We'll let each thread have its own, could be useful in the future
        rest_time.tv_sec = 0;
        rest_time.tv_nsec = 500000;                                             // Half a millisecond seems reasonable to start with.  Also, from the nanosleep
                                                                                // man page:
                                                                                // "If the interval specified in req is not an exact multiple of the granularity
                                                                                // underlying clock (see time(7)), then the interval will be rounded up to the
                                                                                // next multiple."
                                                                                // This is OK.
        
        if( bqt::isInitTaskSystem() )
        {
            {
                bqt::scoped_lock slock( task_thread_count_mutex );
                task_thread_count++;
            }
            
            try
            {
                bool running = true;
                bqt::task* current_task;
                
                while( running )
                {
                    // if( !bqt::getDevMode() || bqt::arrestHook() )               // Arrest hooks only available in dev mode
                    if( true )                                                  // TODO: implement arrest hooks
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
                        }
                        else
                            running = false;                                    // Popped a null task so exit
                        
                        nanosleep( &rest_time, NULL );
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
                ff::write( std::cerr, e.what() );
                ff::write( bqt_out, e.what() );
                
                code = bqt::EXITCODE_BQTERR;
            }
            catch( std::exception& e )
            {
                ff::write( std::cerr, e.what() );
                ff::write( bqt_out, e.what() );
                
                code = bqt::EXITCODE_STDERR;
            }
            
            {
                bqt::scoped_lock slock( task_thread_count_mutex );
                task_thread_count--;
            }
            
            return code;
        }
        else
            throw bqt::exception( "bqt::taskThread(): Task system not initialized" );
    }
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool initTaskSystem( bool main_is_tasker )
    {
        long thread_count = getSystemCoreCount();
        
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
        
        for( int i = 0; i < tc; i++ )
        {
            task_threads_masks[i] = TASK_ALL;
            
            task_threads_data[i].queue = global_task_queue;
            task_threads_data[i].mask  = &( task_threads_masks[i] );
            
            task_threads[i].setFunction( &taskThread );
            task_threads[i].setData( ( void* )( &( task_threads_data[i] ) ) );
            task_threads[i].start();
        }
        
        global_task_queue -> open();
        
        return true;
    }
    bool isInitTaskSystem()
    {
        // Ugh...
        
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
        global_task_queue -> close();                                               // Task queue will now pop NULLs
        
        for( int i = 0; i < spawned_task_thread_count; i++ )
        {
            exit_code ec = task_threads[i].wait();
            
            if( getDevMode() )
                ff::write( bqt_out, "Worker thread exited with code ", exc2str( ec ), "\n" );
        }
    }
    
    bool deInitTaskSystem()
    {
        delete[] task_threads;
        delete[] task_threads_data;
        
        task_threads = NULL;
        task_threads_data = NULL;
    }

    /******************************************************************************//******************************************************************************/

    bool StopTaskSystem_task::execute( task_mask* caller_mask )
    {
        if( ( *caller_mask ) & TASK_TASK )                                          // A little sanity check
            stopTaskSystem();
        else
            throw exception( "StopTaskSystem_task::execute(): Calling thread's mask does not contain TASK_TASK" );
        
        return true;
    }
    task_mask StopTaskSystem_task::getMask()
    {
        return TASK_TASK;
    }

    /******************************************************************************//******************************************************************************/

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
        scoped_lock slock( task_thread_count_mutex );
        return task_thread_count;
    }
}


