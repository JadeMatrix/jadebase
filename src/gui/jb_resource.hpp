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

#include "../dynamics/jb_dpi.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // jade::window predeclared in jb_dpi.hpp
    
    class gui_resource
    {
    public:
        // TODO: Figure out which dpi:: things this should use
        gui_resource( unsigned int,                                               // Width
                      unsigned int );                                             // Height
        virtual ~gui_resource();
        
        virtual std::pair< unsigned int, unsigned int > getDimensions();            // Non-pure virtual, as we are always guaranteed dimensions
        
        virtual void draw( window* ) = 0;
        
    protected:
        unsigned int dimensions[ 2 ];
    };
}

/******************************************************************************//******************************************************************************/

#endif


