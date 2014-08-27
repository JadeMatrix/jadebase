#ifndef BQT_SCOPEDLOCK_HPP
#define BQT_SCOPEDLOCK_HPP

/* 
 * bqt_scopedlock.hpp
 * 
 * Template class for scope-locking various thread safety mechanisms
 * 
 */

/******************************************************************************//******************************************************************************/

namespace bqt
{
    template< class LOCKABLE > class scoped_lock                                // Works for bqt::mutex and anything else that has lock()/unlock() methods
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


