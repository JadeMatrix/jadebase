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
#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class tensor_engine
    {
    public:
        tensor_engine() {};
        virtual ~tensor_engine() {};
        
        tensor< T >* newTensor( solver< T > );
        tensor< T >* newTensor( solver< T >,
                                T );
        tensor< T >* newTensor( solver< T >,
                                tensor_data::storage< T > );
        tensor< T >* newTensor( solver< T >,
                                T,
                                tensor_data::storage< T > );
        
        virtual void solve();                                                   // Virtual as different engines may have different solving algorithms or 
                                                                                // optimization heuristics
        
    protected:
        mutex engine_mutex;
        struct tensor_store
        {
            tensor< T > t;
            bool active;
            
            tensor_store( solver< T >& s ) : t( s ), active( true ) {}
        };
        
        std::vector< tensor_store > tensors;
    };
    
    template< typename T > tensor< T >* tensor_engine::newTensor( solver< T > s )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        for( i =0; i < tensor_count; ++i )
            if( !tensors[ i ].active )
            {
                tensors[ i ].solve = s;
                tensors[ i ].active = true;
                return;
            }
        tensors.push_back( tensor_store( s ) );
        // FIXME: std::vector can't guarantee pointers stay the same.  Return index?
    }
    template< typename T > tensor< T >* tensor_engine::newTensor( solver< T > s,
                                                                  T c )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        for( i =0; i < tensor_count; ++i )
            if( !tensors[ i ].active )
            {
                tensors[ i ].solve = s;
                tensors[ i ].active = true;
                return;
            }
        tensors.push_back( tensor_store( s ) );
        tensors[ tensor_count ].cache = c;
        // FIXME: std::vector can't guarantee pointers stay the same.  Return index?
    }
    template< typename T > tensor< T >* tensor_engine::newTensor( solver< T > s,
                                                                  tensor_data::storage< T > d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        for( i =0; i < tensor_count; ++i )
            if( !tensors[ i ].active )
            {
                tensors[ i ].solve = s;
                tensors[ i ].active = true;
                return;
            }
        tensors.push_back( tensor_store( s ) );
        tensors[ tensor_count ].data = d;
        // FIXME: std::vector can't guarantee pointers stay the same.  Return index?
    }
    template< typename T > tensor< T >* tensor_engine::newTensor( solver< T > s,
                                                                  T c,
                                                                  tensor_data::storage< T > d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        for( i =0; i < tensor_count; ++i )
            if( !tensors[ i ].active )
            {
                tensors[ i ].solve = s;
                tensors[ i ].active = true;
                return;
            }
        tensors.push_back( tensor_store( s ) );
        tensors[ tensor_count ].cache = c;
        tensors[ tensor_count ].data = d;
        // FIXME: std::vector can't guarantee pointers stay the same.  Return index?
    }
    
    template< typename T > void tensor_engine::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        for( i =0; i < tensor_count; ++i )
            if( tensors[ i ].active )
                tensors[ i ].solve( *this, nullptr );
    }
}

/******************************************************************************//******************************************************************************/

#endif


