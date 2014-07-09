/* 
 * bqt_taskqueue.cpp
 * 
 * Implementes bqt::task_queue from bqt_taskqueue.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_taskqueue.hpp"

#include "bqt_exception.hpp"
#include "bqt_log.hpp"
#include "bqt_launchargs.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    task_queue::task_queue()
    {
        status = PREOPEN;
    }
    // task_queue::~task_queue()
    // {
    // }
    
    task* task_queue::pop()
    {
        return pop( TASK_ALL );
    }
    task* task_queue::pop( task_mask mask )
    {
        task* r = NULL;
        bool popping = true;
        
        scoped_lock slock( tq_mutex );
        
        while( popping )
        {
            if( status == CLOSED )
                popping = false;
            else
            {
                if( status == PREOPEN
                    || ( data[ 0 ].empty()
                         && data[ 1 ].empty()
                         && data[ 2 ].empty() ) )
                    tq_cond.wait( tq_mutex );
                else
                {
                    for( int i = 0; i < 3; i++ )                                // Iterate through priority levels
                    {
                        for( std::list< task* >::iterator iter = data[ i ].begin();
                             iter != data[ i ].end();
                             iter++ )                                           // Iterate through tasks in priority level
                        {
                            if( ( *iter ) -> matchMask( mask ) )
                            {
                                r = *iter;
                                data[ i ].erase( iter );
                                popping = false;
                                break;
                            }
                        }
                        
                        if( !popping )
                            break;
                    }
                    
                    if( popping )                                               // If we didn't find anything this time through, try the whole process again
                        tq_cond.wait( tq_mutex );                               // when something's inserted (or the queue closes)
                }
            }
        }
        
        return r;
    }

    void task_queue::push( task* item )
    {
        if( item == NULL )
            throw exception( "task_queue::push(): Cannot push a null task" );
        
        {
            scoped_lock slock( tq_mutex );
            
            if( status == OPEN )
            {
                switch( item -> getPriority() )
                {
                case PRIVAL_HIGH:
                    data[ 0 ].push_back( item );
                    break;
                case PRIVAL_NONE:
                    data[ 1 ].push_back( item );
                    break;
                case PRIVAL_LOW:
                    data[ 2 ].push_back( item );
                    break;
                default:
                    throw exception( "task_queue::push(): Invalid priority level" );
                }
            }
            else
            {
                ff::write( bqt_out, "Warning: task_queue::push(): Queue closed, deleting pushed task\n" );
                delete item;
            }
        }
        
        tq_cond.broadcast();                                                    // Broadcast so all threads can match against mask
    }

    void task_queue::open()
    {
        {
            scoped_lock slock( tq_mutex );
            status = OPEN;
        }
        
        tq_cond.broadcast();
    }
    void task_queue::close()
    {
        {
            scoped_lock slock( tq_mutex );
            
            status = CLOSED;
            
            for( int i = 0; i < 3; i++ )
            {
                for( std::list< task* >::iterator iter = data[ i ].begin();
                     iter != data[ i ].end();
                     iter++ )
                {
                    delete *iter;                                               // Don't use list::erase(), it invalidates the iterator
                }
            }
        }
        
        tq_cond.broadcast();
    }

    int task_queue::size()
    {
        scoped_lock slock( tq_mutex );
        return ( data[ 0 ].size() + data[ 1 ].size() + data[ 2 ].size() );
    }
}


