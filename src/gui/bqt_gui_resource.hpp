#ifndef BQT_GUI_RESOURCE_HPP
#define BQT_GUI_RESOURCE_HPP

/* 
 * bqt_gui_resource.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <utility>

#include "../bqt_window.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class gui_resource
    {
    protected:
        window& parent;
        
        unsigned int dimensions[ 2 ];
    public:
        gui_resource( window& p,
                      unsigned int w,
                      unsigned int h );
        virtual ~gui_resource();
        
        virtual std::pair< unsigned int, unsigned int > getDimensions();
        
        virtual void draw() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


