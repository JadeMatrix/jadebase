#ifndef JADEBASE_GUI_DYNAMICS_HPP
#define JADEBASE_GUI_DYNAMICS_HPP

/* 
 * jb_template.hpp
 * 
 * TODO: Pass in a clock that can be controlled separately
 * TODO: Inherit from tensor_engine to separate animation & constraint storage?
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <chrono>

#include "jb_tensor.hpp"
#include "jb_tensor_engine.hpp"
#include "../utility/jb_dpi.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    typedef std::chrono::high_resolution_clock::time_point tick;                // Simple utility typedef because the fully-qualified name is rediculous
    // typedef std::chrono::high_resolution_clock::duration duration;
    
    class gui_tensor_engine;
    typedef tensor< dpi::points, gui_tensor_engine > gui_tensor;
    
    class gui_tensor_engine
    {
    public:
        typedef unsigned long tensor_id;
        enum tensor_category
        {
              ACTIVE_ANIMATION  = 0x01, // 0001
            INACTIVE_ANIMATION  = 0x02, // 0010
              ACTIVE_CONSTRAINT = 0x03, // 0011
            INACTIVE_CONSTRAINT = 0x04  // 0100
        };                                                                      // The enum is given these specific values so we can just use 0x01 as a bitmask
                                                                                // to check whether the category is an active one.
        
        gui_tensor_engine();
        
        const dpi::points& operator[]( const tensor_id& );
        
        tensor_id newTensor( const gui_tensor& );
        tensor_id newTensor( const gui_tensor&, const tensor_id& );
        
        void releaseTensor( const tensor_id& );
        
        void solve();
        void solveAnimations();
        void solveConstraints();
        
        std::chrono::high_resolution_clock::time_point getTime();
        std::chrono::high_resolution_clock::duration getInterval();                                                     // WARNING: This is NOT guaranteed to be non-zero
    protected:
        mutex engine_mutex;
        
        struct tensor_store
        {
            gui_tensor t;
            dpi::points cache;
            
            bool empty;
            
            tensor_store( const gui_tensor& t ) : t( t ), empty( false ) {}
        };
        
        typedef typename std::vector< tensor_store >::size_type tensor_index;
        
        std::vector< tensor_store > tensors;
        std::map< tensor_id, tensor_index > id_map;
        
        tensor_index free_count;
        tensor_id latest_id;
        
        tick now_tick;
        tick last_tick;
    };
}

/******************************************************************************//******************************************************************************/

#endif


