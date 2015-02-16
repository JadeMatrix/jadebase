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
    protected:
        unsigned int dimensions[ 2 ];
    public:
        gui_resource( unsigned int w,
                      unsigned int h );
        virtual ~gui_resource();
        
        virtual std::pair< unsigned int, unsigned int > getDimensions();
        
        virtual void draw() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


