#ifndef JADEBASE_SEMAPHORE_HPP
#define JADEBASE_SEMAPHORE_HPP

/* 
 * jb_semaphore.hpp
 * 
 * Contains jade::semaphore, a class definition for manipulating & managing
 * semaphores as objects.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_threadutil.hpp"
#include "jb_condition.hpp"
#include "jb_mutex.hpp"
#include "jb_scopedlock.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
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
        
        void acquire( unsigned int count = 1 ) const;
        void acquireAll() const;
        
        void release( unsigned int count = 1 ) const;
        void releaseAll() const;
        
        void increase( unsigned int count = 1 );
        
        // TODO: operators instead?
    };
    
    /* SCOPED_LOCK SPECIALIZATION *********************************************//******************************************************************************/
    
    template<> class scoped_lock< semaphore >
    {
    private:
        const semaphore& sls;
        unsigned int count;
    public:
        scoped_lock( const semaphore& m ) : sls( m ), count( 0 )
        {
            sls.acquireAll();
        }
        scoped_lock( const semaphore& m, unsigned int c ) : sls( m ), count( c )
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


