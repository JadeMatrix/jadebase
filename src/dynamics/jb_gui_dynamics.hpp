#ifndef JADEBASE_GUI_DYNAMICS_HPP
#define JADEBASE_GUI_DYNAMICS_HPP

/* 
 * jb_template.hpp
 * 
 * About
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
    // CONSIDER: Private inheritance
    class gui_tensor_engine : public tensor_engine< dpi::points >
    {
    public:
        void solve();
    protected:
        std::chrono::high_resolution_clock::time_point last_tick;
    };
    
    typedef tensor< dpi::points > gui_tensor;
}

/******************************************************************************//******************************************************************************/

#endif


