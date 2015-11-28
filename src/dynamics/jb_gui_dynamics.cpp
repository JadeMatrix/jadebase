/* 
 * jb_gui_dynamics.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_gui_dynamics.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    gui_tensor_engine::gui_tensor_engine()
    {
        now_tick = std::chrono::high_resolution_clock::now();
    }
    
    const dpi::points& gui_tensor_engine::operator[]( const tensor_id& id )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( id );
        
        if( finder != id_map.end() )
            return tensors[ finder -> second ].cache;
        else
        {
            exception e;
            
            ff::write( *e,
                       "gui_tensor_engine<>::operator[](): No tensor matching tensor ID 0X",
                       ff::to_X( id ) );
            
            throw e;
        }
    }
    
    gui_tensor_engine::tensor_id gui_tensor_engine::newTensor( const gui_tensor& t )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index tensor_count = tensors.size();
        
        if( free_count )
        {
            for( tensor_index i = 0; i < tensor_count; ++i )
                if( tensors[ i ].empty )
                {
                    // tensors[ i ].t = t;
                    new( &tensors[ i ].t ) tensor_store( t );
                    tensors[ i ].empty = false;
                    --free_count;
                    id_map[ ++latest_id ] = i;
                    
                    tensors[ i ].cache = t.solve( tensors[ i ].t, *this );
                    
                    return latest_id;
                }
            
            throw exception( "gui_tensor_engine<>::newTensor(): No free slots but free count non-zero" );
        }
        else
        {
            tensors.push_back( tensor_store( t ) );
            id_map[ ++latest_id ] = tensor_count;
            return latest_id;
        }
    }
    gui_tensor_engine::tensor_id gui_tensor_engine::newTensor( const gui_tensor& t, const gui_tensor_engine::tensor_id& o )
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        auto finder = id_map.find( o );
        
        if( finder != id_map.end() )
        {
            // tensors[ finder -> second ].t = t;
            new( &tensors[ finder -> second ].t ) tensor_store( t );
            
            return o;
        }
        else
        {
            exception e;
            
            ff::write( *e,
                       "gui_tensor_engine<>::newTensor() replace: No tensor matching tensor ID 0X",
                       ff::to_X( o ) );
            
            throw e;
        }
    }
    
    void gui_tensor_engine::releaseTensor( const gui_tensor_engine::tensor_id& id )
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
                       "gui_tensor_engine<>::releaseTensor(): No tensor matching tensor ID 0X",
                       ff::to_X( id ) );
            
            throw e;
        }
    }
    
    void gui_tensor_engine::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        last_tick = now_tick;
        now_tick = std::chrono::high_resolution_clock::now();
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i = 0; i < tensor_count; ++i )
            if( !tensors[ i ].empty
                && tensors[ i ].t.solve != nullptr
                && tensors[ i ].t.category & 0x01 )
            {
                tensors[ i ].cache = tensors[ i ].t.solve( tensors[ i ].t, *this );
            }
    }
    void gui_tensor_engine::solveAnimations()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        last_tick = now_tick;
        now_tick = std::chrono::high_resolution_clock::now();
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i = 0; i < tensor_count; ++i )
            if( !tensors[ i ].empty
                && tensors[ i ].t.solve != nullptr
                && tensors[ i ].t.category == ACTIVE_ANIMATION )
            {
                tensors[ i ].cache = tensors[ i ].t.solve( tensors[ i ].t, *this );
            }
    }
    void gui_tensor_engine::solveConstraints()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i = 0; i < tensor_count; ++i )
            if( !tensors[ i ].empty
                && tensors[ i ].t.solve != nullptr
                && tensors[ i ].t.category == ACTIVE_CONSTRAINT )
            {
                tensors[ i ].cache = tensors[ i ].t.solve( tensors[ i ].t, *this );
            }
    }
    
    std::chrono::high_resolution_clock::time_point gui_tensor_engine::getTime()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        return now_tick;
    }
    std::chrono::high_resolution_clock::duration gui_tensor_engine::getInterval()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        return now_tick - last_tick;
    }
}


