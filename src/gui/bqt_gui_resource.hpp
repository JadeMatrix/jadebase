#ifndef BQT_GUI_RESOURCE_HPP
#define BQT_GUI_RESOURCE_HPP

/* 
 * bqt_gui_resource.hpp
 * 
 * Wrapper for bitmap GUI resources
 * 
 * gui_resource contains no thread-safety as after initialization it is read-
 * only.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_layout.hpp"
#include "bqt_gui_texture.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class gui_resource
    {
    protected:
        layout& parent;
        unsigned int dimensions[ 2 ];                                           // Relative dimensions
        unsigned int   position[ 2 ];                                           // Relative position
        
        gui_texture* texture;
    public:
        gui_resource( layout& p,
                      std::string f,
                      unsigned int x,
                      unsigned int y,
                      unsigned int w,
                      unsigned int h );
        ~gui_resource();
        
        std::pair< unsigned int, unsigned int > getDimensions();
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


