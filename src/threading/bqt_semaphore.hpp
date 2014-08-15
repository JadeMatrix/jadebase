#ifndef BQT_SEMAPHORE_HPP
#define BQT_SEMAPHORE_HPP

/* 
 * bqt_semaphore.hpp
 * 
 * Contains bqt::semaphore, a class definition for manipulating & managing
 * semaphores as objects.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_threadutil.hpp"
#include "bqt_condition.hpp"
#include "bqt_mutex.hpp"
#include "bqt_scopedlock.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class semaphore
    {
    protected:
        unsigned int start;
        unsigned int free;
        condition s_cond;
        mutex s_mutex;
    public:
        semaphore( unsigned int start = 0 );
        
        void acquire( unsigned int count = 1 );
        void acquireAll();
        
        void release( unsigned int count = 1 );
        void releaseAll();
        
        void increase( unsigned int count = 1 );
        
        // TODO: operators instead?
    };
    
    /* SCOPED_LOCK SPECIALIZATION *********************************************//******************************************************************************/
    
    template<> class scoped_lock< semaphore >
    {
    private:
        semaphore& sls;
        unsigned int count;
    public:
        scoped_lock( semaphore& m ) : sls( m ), count( 0 )
        {
            sls.acquireAll();
        }
        scoped_lock( semaphore& m, unsigned int c ) : sls( m ), count( c )
        {
            sls.acquire( count );
        }
        ~scoped_lock()
        {
            if( count )
                sls.release( count );
            else
                sls.releaseAll();
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


