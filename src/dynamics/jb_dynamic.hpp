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

// #include "jb_dynamic_guard.hpp"
#include "../utility/jb_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class window;
    
    template< typename T > class dynamic
    {
    public:
        std::vector< dynamic< T > > subs;
        
        dynamic( window& );
        dynamic( const dynamic< T >& );
        virtual ~dynamic();
        
        void changeContext( window& );
        window& getContext();
        
        void perform( T& );
    private:
        virtual void twist( T& ) = 0;
    };
    
    template< typename New, typename Type, typename Dynamic = dynamic< Type > > class dynamic_inherit : Dynamic
    {
    protected:
        dynamic_inherit( window& c ) : Dynamic( c )
        {
            static_assert( sizeof( New ) == sizeof( Dynamic ),
                           "Classes inheriting from jade::dynamic may not add member variables" );
        }
    };
    
    template< typename T > dynamic< T >::dynamic( window& c )
    {
        // TODO: Implement
    }
    template< typename T > dynamic< T >::dynamic( const dynamic< T >& o ) : subs( o.subs )
    {
        // TODO: Implement
    }
    
    template< typename T > void dynamic< T >::changeContext( window& c )
    {
        // TODO: Implement
    }
    template< typename T > window& dynamic< T >::getContext()
    {
        // TODO: Implement
    }
    
    template< typename T > void dynamic< T >::perform( T& t )
    {
        twist( t );
        for( int i = 0; i < subs.length(); ++i )
            subs[ i ].perform( t );
    }
    
    // template< typename T > class dynamic
    // {
    //     JB_DYNAMIC
    // public:
    //     typedef void ( * twister )( T& );
        
    //     std::vector< dynamic< T > > subs;                                       // Leaving this public is the simplest way of manipulating it
    //     twister twist;
        
    //     dynamic( window& c );
    //     dynamic( const dynamic< T >& o );
    //     virtual ~dynamic() {}
        
    //     void changeContext( window& );                                          // Wrapping the context and passing it as a reference to discourage NULLifying
    //     window& getContext();
        
    //     void get( T& );
    // };
    
    // template< typename T > dynamic< T >::dynamic( window& c )
    // {
    //     // _dynamic_private_change_context( ( void* )this, &c );
    // }
    // template< typename T > dynamic< T >::dynamic( const dynamic< T >& o ) : subs( o.subs )
    // {
    //     // context = &c;
    //     // _dynamic_private_change_context( ( void* )this, &c );
    // }
    
    // template< typename T > void dynamic< T >::changeContext( window& c )
    // {
    //     // context = &c;
    //     // _dynamic_private_change_context( ( void* )this, &c );
    // }
    
    // template< typename T > void dynamic< T >::get( T& start )
    // {
    //     _dynamic_private_assert_size();
        
    //     if( twist != nullptr )
    //         twist( start );
        
    //     for( int i = 0; i < subs.length(); ++i )
    //         subs[ i ] -> get( start );
    // }
    
    class DynamicsTestPass_dynamic : public dynamic_inherit< DynamicsTestPass_dynamic, float >
    {
    public:
        DynamicsTestPass_dynamic( window& c ) : dynamic_inherit( c ) {}
    private:
        void twist( float& foo )
        {
            foo *= 2.0f;
        }
    };
    
    // class DynamicsTestPassFail_dynamic : public dynamic_inherit< DynamicsTestPassFail_dynamic >
    
    class DynamicsTestFail_dynamic : public dynamic_inherit< DynamicsTestFail_dynamic, float >
    {
    public:
        DynamicsTestFail_dynamic( window& c ) : dynamic_inherit( c ) {}
    private:
        float bar;
        void twist( float& foo )
        {
            foo *= 2.0f;
        }
    };
    
    // DynamicsTestPass_dynamic pass_test;
    // DynamicsTestFail_dynamic fail_test;
}

/******************************************************************************//******************************************************************************/

#endif


