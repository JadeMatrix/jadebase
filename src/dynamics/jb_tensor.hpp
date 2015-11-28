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
    template< typename T, class E > struct tensor;
    template< typename T > class basic_tensor_engine;                           // Forward declaration of basic_tensor_engine - see jb_tensor_engine.hpp
    
    template< typename T,
              class E = basic_tensor_engine< T > > using solver = T ( * )( tensor< T, E >&,
                                                                           E& );
    
    template< typename T, class E = basic_tensor_engine< T > > struct tensor
    {
        const solver< T, E > solve;                                             // A tensor's solve may not overwrite its own solve
        
        union
        {
            void* p[ 2 ];
            T d[ 2 * sizeof( void* ) / sizeof( T ) ];
            // CONSIDER: Storage type of a typedef from E
        } priv;                                                                 // Private data for this tensor type
        
        typename E::tensor_id ref[ 2 ];                                         // Up to two tensor IDs as references to other tensors; these are dedicated
                                                                                // fields so tensor engines can detect circular dependencies
        typename E::tensor_category category;
        
        tensor( solver< T, E > s ) : solve( s ) {}                              // Constructor must take a single solver< T > as solve is const
        tensor( const tensor< T, E >& o ) : solve( o.solve )
        {
            priv = o.priv;
            ref[ 0 ] = o.ref[ 0 ];
            ref[ 1 ] = o.ref[ 1 ];
            category = o.category;
        }
    };
}

/******************************************************************************//******************************************************************************/

#endif


