#ifndef JADEBASE_RESOURCE_HPP
#define JADEBASE_RESOURCE_HPP

/* 
 * jb_resource.hpp
 * 
 * Base class for all gui resources
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <utility>

#include "../utility/jb_dpi.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // jade::window predeclared in jb_dpi.hpp
    
    class gui_resource
    {
    public:
        gui_resource( dpi::points,                                              // Width
                      dpi::points );                                            // Height
        virtual ~gui_resource();
        
        virtual std::pair< dpi::points, dpi::points > getDimensions();          // Non-pure virtual, as we are always guaranteed dimensions
        
        virtual void draw( window* ) = 0;
        
    protected:
        dpi::points dimensions[ 2 ];
    };
}

/******************************************************************************//******************************************************************************/

#endif


