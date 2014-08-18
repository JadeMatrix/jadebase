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
#include "bqt_gui_texture.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class gui_resource
    {
    protected:
        window& parent;
        gui_texture* texture;
        
        GLuint gl_dlist;
        bool dlist_created;
        
        unsigned int   position[ 2 ];
        unsigned int dimensions[ 2 ];
        
        void createDisplayList();
    public:
        gui_resource( window& parent,
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


