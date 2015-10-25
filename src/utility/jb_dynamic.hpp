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
 * dynamic may be supplied with an optional starting value of type T on get().
 * However, sub-dynamics' twist()s are not required to respect the value passed
 * in, and may change it to a completely unrelated value.  The validity of the
 * dynamics chain semantics is up to the chain implementer.
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
        virtual dynamic( window& ) : context( &c ) {}
        virtual ~dynamic() {}
        
        void addSubDynamic( dynamic< T >& );
        void changeContext( window& );
        
        const T& get();
        const T& get( T& );
    protected:
        window* context;
        std::vector< dynamic< T > > sub_dynamics;
        
        virtual void twist( T& ) = 0;
    };
    
    template< typename T > void dynamic< T >::addSubDynamic( dynamic< T >& s )
    {
        sub_dynamics.push_back( s );
    }
    template< typename T > void dynamic< T >::changeContext( window& c )
    {
        context = &t;
    }
    
    template< typename T > const T& dynamic< T >::get()
    {
        T uninitialized;
        return get( uninitialized );
    }
    template< typename T > const T& dynamic< T >::get( T& start )
    {
        twist( start );
        
        for( int i = 0; i < sub_dynamics.length(); ++i )
            sub_dynamics[ i ] -> twist( start );
        
        return start;
    }
}

/******************************************************************************//******************************************************************************/

#endif


