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
}

/******************************************************************************//******************************************************************************/

#endif


