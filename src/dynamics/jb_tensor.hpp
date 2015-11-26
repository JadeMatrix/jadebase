#ifndef JADEBASE_TENSOR_HPP
#define JADEBASE_TENSOR_HPP

/* 
 * jb_tensor.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // typedef unsigned long tensor_id;
    template< typename T > using tensor_id = unsigned long;
    
    template< typename T > struct tensor_data
    {
        struct storage
        {
            tensor_id< T > ref[ 2 ];
            
            union
            {
                void* p[ 2 ];
                T d[ 2 * sizeof( void* ) / sizeof( T ) ];
            } priv;                                                             // Private data for this tensor type
        };
        
        T cache;
    };
    
    template< typename T > using solver = void ( * )( tensor_data< T >&,
                                                      void* );
    
    template< typename T > class tensor final
    {
        template< typename > friend class tensor_engine;
        
    public:
        const T operator*() const;
        
        void solve_wrap( void* );
        
    protected:
        mutex tensor_mutex;
        solver< T > solve;
        tensor_data< T > data;
        
        tensor( solver< T > );
    };
    
    // template< typename T > void default_tensor_solver( tensor_data< T >& ) {}
    
    template< typename T > const T tensor< T >::operator*() const
    {
        scoped_lock< mutex > slock( tensor_mutex );
        return data.cache;
    }
    
    template< typename T > tensor< T >::tensor( solver< T > s )
    {
        solve = s;
    }
    
    template< typename T > void tensor< T >::solve_wrap( void* engine_data )
    {
        if( solve )
        {
            scoped_lock< mutex > slock( tensor_mutex );
            solve( data, engine_data );
        }
    }
}

/******************************************************************************//******************************************************************************/

#endif


