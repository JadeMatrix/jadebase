/* 
 * bqt_semaphore.cpp
 * 
 * Implements bqt::semaphore from bqt_semaphore.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_semaphore.hpp"

#include "../bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    semaphore::semaphore( unsigned int start )
    {
        this -> start = start;
        free = start;
    }

    void semaphore::acquire( unsigned int count )
    {
        try
        {
            scoped_lock s_lock( s_mutex );
        }
        catch( exception& e )
        {
            throw exception( "semaphore::acquire(): " + std::string( e.what() ) );
        }
        
        if( count > start )
            throw exception( "semaphore::acquire(): Attempt to acquire higher count than possibly available" );
        else
            while( count > 0 )
            {
                if( free < count )
                {
                    count -= free;
                    free = 0;
                    
                    s_cond.wait( s_mutex );
                }
                else
                {
                    free -= count;
                    // count = 0;
                    break;
                }
            }
    }
    void semaphore::acquireAll()
    {
        acquire( start );
    }

    void semaphore::release( unsigned int count )
    {
        try
        {
            scoped_lock s_lock( s_mutex );
        }
        catch( exception& e )
        {
            throw exception( "semaphore::release(): " + std::string( e.what() ) );
        }
        
        if( ( free + count ) > start )
            throw exception( "semaphore::release(): Attempt to release more than possibly available" );
        else
        {
            free += count;
            s_cond.signal();
        }
    }
    void semaphore::releaseAll()
    {
        release( start );
    }

    void semaphore::increase( unsigned int count )
    {
        try
        {
            scoped_lock s_lock( s_mutex );
        }
        catch( exception& e )
        {
            throw exception( "semaphore::increase(): " + std::string( e.what() ) );
        }
        
        if( ( start + count ) < start )
            throw exception( "semaphore::increase(): Overflow" );
        else
        {
            start += count;
            s_cond.signal();
        }
    }
}


