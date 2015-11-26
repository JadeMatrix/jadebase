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
#include <map>

#include "jb_tensor.hpp"
#include "../threading/jb_mutex.hpp"
#include "../utility/jb_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class tensor_engine
    {
    public:
        tensor_engine();
        virtual ~tensor_engine() {};
        
        tensor_id< T > newTensor( solver< T > );
        tensor_id< T > newTensor( solver< T >,
                                  T );
        tensor_id< T > newTensor( solver< T >,
                                  tensor_data::storage< T > );
        tensor_id< T > newTensor( solver< T >,
                                  T,
                                  tensor_data::storage< T > );
        
        void releaseTensor( tensor_id< T > );
        
        tensor< T >& operator[]( tensor_id< T > );
        
        virtual void solve();                                                   // Virtual as different engines may have different solving algorithms or 
                                                                                // optimization heuristics
        
    protected:
        mutex engine_mutex;
        struct tensor_store
        {
            tensor< T > t;
            bool empty;
            bool active;
            
            tensor_store( solver< T >& s ) : t( s ),
                                             active( true ),
                                             empty( false ) {}
        };
        
        typedef std::vector< tensor_store >::size_type tensor_index;
        
        bool free_count;
        std::vector< tensor_store > tensors;
        tensor_id< T > latest_id;
        std::map< tensor_id< T >, tensor_index > id_map;
        
        tensor_index insertBasic( solver< T > );
    };
    
    template< typename T > tensor_engine::tensor_engine()
    {
        latest_id = 0x00;
        free_count = 0;
    }
    
    template< typename T > tensor_id< T > tensor_engine::newTensor( solver< T > s )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        return insertBasic( s );
    }
    template< typename T > tensor_id< T > tensor_engine::newTensor( solver< T > s,
                                                                    T c )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        tensors[ i ].cache = c;
        
        return i;
    }
    template< typename T > tensor_id< T > tensor_engine::newTensor( solver< T > s,
                                                                    tensor_data::storage< T > d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        // TODO: Check for circular references
        
        tensors[ i ].data = d;
        
        return i;
    }
    template< typename T > tensor_id< T > tensor_engine::newTensor( solver< T > s,
                                                                    T c,
                                                                    tensor_data::storage< T > d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        // TODO: Check for circular references
        
        tensors[ i ].cache = c;
        tensors[ i ].data = d;
        
        return i;
    }
    
    template< typename T > void tensor_engine::releaseTensor( tensor_id< T > i )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( i );
        
        if( finder != id_map.end() )
        {
            tensors[ finder -> second ].empty = true;
            id_map.erase( finder );
            ++free_count;
        }
        else
            throw exception( "tensor_engine<>:releaseTensor(): No such tensor ID" );
    }
    
    template< typename T > tensor< T >& tensor_engine::operator[]( tensor_id< T > i )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( i );
        
        if( finder != id_map.end() )
            return tensors[ finder -> second ];
        else
            throw exception( "tensor_engine<>::operator[](): No such tensor ID" );
    }
    
    template< typename T > void tensor_engine::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto tensor_count = tensors.size();
        
        for( i =0; i < tensor_count; ++i )
            if( tensors[ i ].active )
                tensors[ i ].solve( *this, nullptr );
    }
    
    template< typename T > tensor_engine::tensor_index tensor_engine::insertBasic( solver< T > s )
    {
        tensor_index tensor_count = tensors.size();
        
        if( free_count )
            for( tensor_index i = 0; i < tensor_count; ++i )
                if( !tensors[ i ].empty )
                {
                    tensors[ i ].solve = s;
                    tensors[ i ].active = true;
                    tensors[ i ].empty = false;
                    
                    id_map[ ++latest_id ] = i;
                    --free_count;
                    return i;
                }
        
        tensors.push_back( tensor_store( s ) );
        return tensor_count;
    }
}

/******************************************************************************//******************************************************************************/

#endif


