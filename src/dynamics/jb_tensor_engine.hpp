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
                                  typename tensor_data< T >::storage );
        tensor_id< T > newTensor( solver< T >,
                                  T,
                                  typename tensor_data< T >::storage );
        
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
        
        typedef typename std::vector< tensor_store >::size_type tensor_index;
        // using tensor_index = std::vector< tensor_store >::size_type;
        
        bool free_count;
        std::vector< tensor_store > tensors;
        tensor_id< T > latest_id;
        std::map< tensor_id< T >, tensor_index > id_map;
        
        tensor_index insertBasic( solver< T > );
    };
    
    template< typename T > tensor_engine< T >::tensor_engine()
    {
        latest_id = 0x00;
        free_count = 0;
    }
    
    template< typename T > tensor_id< T > tensor_engine< T >::newTensor( solver< T > s )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        return insertBasic( s );
    }
    template< typename T > tensor_id< T > tensor_engine< T >::newTensor( solver< T > s,
                                                                         T c )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        tensors[ i ].t.cache = c;
        
        return i;
    }
    template< typename T > tensor_id< T > tensor_engine< T >::newTensor( solver< T > s,
                                                                         typename tensor_data< T >::storage d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        // TODO: Check for circular references
        
        tensors[ i ].t.data = d;
        
        return i;
    }
    template< typename T > tensor_id< T > tensor_engine< T >::newTensor( solver< T > s,
                                                                         T c,
                                                                         typename tensor_data< T >::storage d )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index i = insertBasic( s );
        
        // TODO: Check for circular references
        
        tensors[ i ].t.cache = c;
        tensors[ i ].t.data = d;
        
        return i;
    }
    
    template< typename T > void tensor_engine< T >::releaseTensor( tensor_id< T > i )
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
    
    template< typename T > tensor< T >& tensor_engine< T >::operator[]( tensor_id< T > i )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( i );
        
        if( finder != id_map.end() )
            return tensors[ finder -> second ].t;
        else
            throw exception( "tensor_engine<>::operator[](): No such tensor ID" );
    }
    
    template< typename T > void tensor_engine< T >::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i =0; i < tensor_count; ++i )
            if( tensors[ i ].active )
                tensors[ i ].t.solve_wrap( ( void* )this );
    }
    
    template< typename T > typename tensor_engine< T >::tensor_index tensor_engine< T >::insertBasic( solver< T > s )
    {
        tensor_index tensor_count = tensors.size();
        
        if( free_count )
            for( tensor_index i = 0; i < tensor_count; ++i )
                if( !tensors[ i ].empty )
                {
                    tensors[ i ].t.solve = s;
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


