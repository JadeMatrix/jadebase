#ifndef JADEBASE_DYNAMIC_HPP
#define JADEBASE_DYNAMIC_HPP

/* 
 * jb_dynamic.hpp
 * 
 * Framework for temporally, spacially, and semantically dynamic values
 * 
 * Dynamics need to operate in contex of a window.  This window can change but
 * must always exist for the dynamic to exist.
 *
 * Dynamics are implicitly nestable.  Classes implementing the base dynamic<>
 * class allow this by overriding the pure virtual twist() function with their
 * specific functionality, which 'twists' the value passed in.  The top-level
 * dynamic must be supplied with a starting value of type T on get().  However,
 * sub-dynamics' twist()s are not required to respect the value passed in, and
 * may change it to a completely unrelated value.  The validity of the dynamics
 * chain semantics is up to the chain implementer.
 * 
 * Dynamics are not guaranteed to be thread-safe, and so rely on the thread-
 * safety of their get()er.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>

/******************************************************************************//******************************************************************************/

namespace jade
{
    class window;
    
    template< typename T > class dynamic
    {
    public:
        std::vector< dynamic< T > > subs;                                       // Leaving this public is the simplest way of manipulating it
        
        dynamic( window& c ) : context( &c ) {}
        dynamic( const dynamic< T >& o ) : context( o.context ), subs( o.subs ) {}
        virtual ~dynamic() {}
        
        void changeContext( window& );
        
        void get( T& );
    protected:
        window* context;
        
        virtual void twist( T& ) = 0;
    };
    
    template< typename T > void dynamic< T >::changeContext( window& c )
    {
        context = &c;
    }
    
    template< typename T > void dynamic< T >::get( T& start )
    {
        twist( start );
        
        for( int i = 0; i < subs.length(); ++i )
            subs[ i ] -> twist( start );
    }
}

/******************************************************************************//******************************************************************************/

#endif


