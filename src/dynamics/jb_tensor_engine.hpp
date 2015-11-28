#ifndef JADEBASE_TENSOR_ENGINE_HPP
#define JADEBASE_TENSOR_ENGINE_HPP

/* 
 * jb_tensor_engine.hpp
 *
 * Supplied here are two class templates, tensor_engine<> and
 * basic_tensor_engine<>.  The former is simply an example of what is expected
 * of tensor engines, and cannot be used by itself.  The latter is a reference
 * implementation that gives a bare minimum of functionality required for
 * generic storage, solving, and retrieval of tensors in a tensor engine.
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
    // Basic interface for a tensor engine /////////////////////////////////////
    
    #if 0
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
    #endif
    
    // Tensor engine reference implementation //////////////////////////////////
    
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
    
    #if 0
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
    #endif
}

/******************************************************************************//******************************************************************************/

#endif


