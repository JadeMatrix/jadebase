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
    template< typename T > struct tensor_data
    {
        union storage
        {
            void* p[ 2 ];
            T d[ 2 * sizeof( void* ) / sizeof( T ) ];
        } private;                                                              // Private data for this tensor type
        
        T cache;
    }
    
    template< typename T > using solver = void ( * )( tensor_data< T >&,
                                                      void* );
    
    template< typename T > class tensor final
    {
        friend class tensor_engine< T >;
        
    public:
        const T operator*() const;
        
    protected:
        mutex tensor_mutex;
        solver< T > solve;
        tensor_data data;
        
        tensor( solver< T > );
        
        void solve_wrap( void* );
    }
    
    // template< typename T > void default_tensor_solver( tensor_data< T >& ) {}
    
    template< typename T > const T tensor::operator*() const
    {
        scoped_lock< mutex > slock( tensor_mutex );
        return cache;
    }
    
    template< typename T > tensor::tensor( solver< T > s )
    {
        solver = s;
    }
    
    template< typename T > void tensor::solve_wrap( void* engine_data )
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


