/* 
 * bqt_gui_resource.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    gui_resource::gui_resource( unsigned int w,
                                unsigned int h )
    {
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        // Empty
    }
    
    std::pair< unsigned int, unsigned int > gui_resource::getDimensions()
    {
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
}


