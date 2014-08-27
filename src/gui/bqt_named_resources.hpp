#ifndef BQT_NAMED_RESOURCE_HPP
#define BQT_NAMED_RESOURCE_HPP

/* 
 * bqt_named_resources.hpp
 * 
 * Utilities for built-in named GUI resources
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource_names.hpp"
#include "bqt_gui_resource.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void initNamedResources();
    void deInitNamedResources();
    
    gui_resource* getNamedResource( gui_resource_name name );
}

/******************************************************************************//******************************************************************************/

#endif


