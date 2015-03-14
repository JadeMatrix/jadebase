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

/******************************************************************************//******************************************************************************/

namespace jade
{
    class gui_resource
    {
    public:
        gui_resource( unsigned int,                                             // Width
                      unsigned int );                                           // Height
        virtual ~gui_resource();
        
        virtual std::pair< unsigned int, unsigned int > getDimensions();        // Non-pure virtual, as we are always guaranteed dimensions
        
        virtual void draw() = 0;
        
    protected:
        unsigned int dimensions[ 2 ];
    };
}

/******************************************************************************//******************************************************************************/

#endif


