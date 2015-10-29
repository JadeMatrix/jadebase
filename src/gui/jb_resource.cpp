/* 
 * jb_resource.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_resource.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    gui_resource::gui_resource( dpi::points w,
                                dpi::points h )
    {
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        // Empty
    }
    
    std::pair< dpi::points, dpi::points > gui_resource::getDimensions()
    {
        return std::pair< dpi::points, dpi::points >( dimensions[ 0 ], dimensions[ 1 ] );
    }
}


