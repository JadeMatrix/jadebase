#ifndef JADEBASE_SCOPEDLOCK_HPP
#define JADEBASE_SCOPEDLOCK_HPP

/* 
 * jb_scopedlock.hpp
 * 
 * Template class for scope-locking various thread safety mechanisms
 * 
 */

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< class LOCKABLE > class scoped_lock                                // Works for jade::mutex and anything else that has lock()/unlock() methods
    {
    private:
        const LOCKABLE& slx;
    public:
        scoped_lock( const LOCKABLE& x ) : slx( x )
        {
            slx.lock();
        }
        ~scoped_lock()
        {
            slx.unlock();
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


