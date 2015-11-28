#ifndef JADEBASE_BASIC_TENSOR_ENGINE_HPP
#define JADEBASE_BASIC_TENSOR_ENGINE_HPP

/* 
 * jb_basic_tensor_engine.hpp
 *
 * Supplied here is a reference implementation that gives a bare minimum of
 * functionality required for generic storage, solving, and retrieval of tensors
 * in a tensor engine.
 *
 * A class should only inherit from basic_tensor_engine<> if its requirements
 * are not far beyond what basic_tensor_engine<> already provides.  If any
 * futher changes must be made, the new class should instead reimplement the
 * interface specified in tensor_engine<> and let duck typing do the rest.
 * 
 * The typedef basic_tensor<> is supplied merely for naming consistency, as the
 * template definition for tensor<> already supplies basic_tensor_engine<> as
 * the default second template argument.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>
#include <map>

#include "jb_tensor.hpp"
#include "../threading/jb_mutex.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    template< typename T > class basic_tensor_engine;
    template< typename T > using basic_tensor = tensor< T, basic_tensor_engine< T > >;
    
    template< typename T > class basic_tensor_engine
    {
    public:
        typedef unsigned long tensor_id;
        enum tensor_category
        {
            ACTIVE,
            INACTIVE
        };
        
        virtual const T& operator[]( const tensor_id& );
        
        virtual tensor_id newTensor( const basic_tensor< T >& );
        virtual tensor_id newTensor( const basic_tensor< T >&, const tensor_id& );
        
        virtual void releaseTensor( const tensor_id& );
        
        virtual void solve();
        
    protected:
        mutex engine_mutex;
        
        struct tensor_store
        {
            basic_tensor< T > t;
            T cache;
            
            bool empty;
            
            tensor_store( basic_tensor< T >& t )
            {
                this -> t = t;
                empty = false;
            }
        };
        
        typedef typename std::vector< tensor_store >::size_type tensor_index;
        
        std::vector< tensor_store > tensors;
        std::map< tensor_id, tensor_index > id_map;
        
        tensor_index free_count;
        tensor_id latest_id;
    };
    
    template< typename T > const T& basic_tensor_engine< T >::operator[]( const tensor_id& id )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( id );
        
        if( finder != id_map.end() )
            return tensors[ finder -> second ].cache;
        else
        {
            exception e;
            
            ff::write( *e,
                       "basic_tensor_engine<>::operator[](): No tensor matching tensor ID 0X",
                       ff::to_X( id ) );
            
            throw e;
        }
    }
    
    template< typename T > typename basic_tensor_engine< T >::tensor_id basic_tensor_engine< T >::newTensor( const basic_tensor< T >& t )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index tensor_count = tensors.size();
        
        if( free_count )
        {
            for( tensor_index i = 0; i < tensor_count; ++i )
                if( tensors[ i ].empty )
                {
                    tensors[ i ].t = t;
                    tensors[ i ].empty = false;
                    --free_count;
                    id_map[ ++latest_id ] = i;
                    
                    tensors[ i ].cache = t.solve( tensors[ i ].t, *this );      // Cache value immediately, making sure we pass a reference to the final tensor;
                                                                                // also cache after setting all other values just in case the solver references
                                                                                // the engine and uses those values (please don't do this).
                    
                    return latest_id;
                }
            
            throw exception( "basic_tensor_engine<>::newTensor(): No free slots but free count non-zero" );
        }
        else
        {
            tensors.push_back( tensor_store( t ) );
            id_map[ ++latest_id ] = tensor_count;
            return latest_id;
        }
    }
    template< typename T > typename basic_tensor_engine< T >::tensor_id basic_tensor_engine< T >::newTensor( const basic_tensor< T >& t,
                                                                                                             const tensor_id& o )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( o );
        
        if( finder != id_map.end() )
        {
            tensors[ finder -> second ].t = t;
            
            return o;
        }
        else
        {
            exception e;
            
            ff::write( *e,
                       "basic_tensor_engine<>::newTensor() replace: No tensor matching tensor ID 0X",
                       ff::to_X( o ) );
            
            throw e;
        }
    }
    
    template< typename T > void basic_tensor_engine< T >::releaseTensor( const basic_tensor_engine< T >::tensor_id& id )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( id );
        
        if( finder != id_map.end() )
        {
            tensors[ finder -> second ].empty = true;
            id_map.erase( finder );
            ++free_count;
        }
        else
        {
            exception e;
            
            ff::write( *e,
                       "basic_tensor_engine<>::releaseTensor(): No tensor matching tensor ID 0X",
                       ff::to_X( id ) );
            
            throw e;
        }
    }
    
    template< typename T > void basic_tensor_engine< T >::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i = 0; i < tensor_count; ++i )
            if( !tensors[ i ].empty
                && tensors[ i ].t.solve != nullptr
                && tensors[ i ].t.category == ACTIVE )
            {
                tensors[ i ].cache = tensors[ i ].t.solve( tensors[ i ].t, *this );
            }
    }
}

/******************************************************************************//******************************************************************************/

#endif


