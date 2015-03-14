#ifndef JADEBASE_NAMED_RESOURCE_HPP
#define JADEBASE_NAMED_RESOURCE_HPP

/* 
 * jb_named_resources.hpp
 * 
 * Utilities for built-in named GUI resources
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_resource.hpp"
#include "jb_resource_names.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void initNamedResources();
    void deInitNamedResources();
    
    gui_resource* getNamedResource( gui_resource_name );                        // Throws an exception if no resource exists with that name
}

/******************************************************************************//******************************************************************************/

#endif


