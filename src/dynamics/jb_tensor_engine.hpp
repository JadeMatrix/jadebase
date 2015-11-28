#ifndef JADEBASE_TENSOR_ENGINE_HPP
#define JADEBASE_TENSOR_ENGINE_HPP

/* 
 * jb_tensor_engine.hpp
 * 
 * Supplied here is jade::tensor_engine<>, a class template meant to serve as an
 * outline to implementing an engine for jade::tensor.  It is simply an example
 * and cannot be used by itself.
 *
 * All tensor engines must have a tensor_id and tensor_category type.  Any class
 * methods can differ from tensor_engine<>, but typically they will follow the
 * pattern presented here.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_tensor.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class tensor_engine final
    {
    public:
        typedef void* tensor_id;
        typedef bool tensor_category;
        
        virtual const T& operator[]( const tensor_id& ) const = 0;
        
        virtual tensor_id newTensor( const tensor< T >& )                   = 0;
        virtual tensor_id newTensor( const tensor< T >&, const tensor_id& ) = 0;
        
        virtual void releaseTensor( const tensor_id& ) = 0;
        
        virtual void solve() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


