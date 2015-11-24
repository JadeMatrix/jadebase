#ifndef JADEBASE_TENSOR_ENGINE_HPP
#define JADEBASE_TENSOR_ENGINE_HPP

/* 
 * jb_tensor_engine.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>

#include "jb_tensor.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class tensor_engine
    {
    public:
        tensor_engine();
        
        tensor< T >* newTensor( solver< T > );
        tensor< T >* newTensor( solver< T >,
                                T );
        tensor< T >* newTensor( solver< T >,
                                tensor_data::storage< T > );
        tensor< T >* newTensor( solver< T >,
                                T,
                                tensor_data::storage< T > );
        
        void solve();
        
    protected:
        struct tensor_store
        {
            tensor< T > asdf;
            bool active;
        };
        
        std::vector< tensor_store > tensors;
    };
}

/******************************************************************************//******************************************************************************/

#endif


