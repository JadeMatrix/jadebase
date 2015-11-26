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
    void gui_tensor_engine::solve()
    {
        scoped_lock< mutex > slock( engine_mutex );
        
        // We might want to do delta-t eventually, but for now it's absolute-t
        // and tensors that need deltas keep their own.
        auto new_tick = std::chrono::high_resolution_clock::now();
        
        tensor_index tensor_count = tensors.size();
        
        for( tensor_index i =0; i < tensor_count; ++i )
            if( tensors[ i ].active )
                tensors[ i ].t.solve_wrap( ( void* )( &new_tick ) );
        
        last_tick = new_tick;
    }
}


