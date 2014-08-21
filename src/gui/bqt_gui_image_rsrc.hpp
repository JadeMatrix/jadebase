#ifndef BQT_GUI_RSRC_IMAGE_HPP
#define BQT_GUI_RSRC_IMAGE_HPP

/* 
 * bqt_gui_imaged_rsrc.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource.hpp"
#include "bqt_gui_texture.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class image_rsrc : public gui_resource
    {
    protected:
        gui_texture* texture;
        
        GLuint gl_dlist;
        bool dlist_created;
        
        unsigned int tex_pos[ 2 ];
        
        void createDisplayList();
    public:
        image_rsrc( window& parent,
                    std::string f,
                    unsigned int x,
                    unsigned int y,
                    unsigned int w,
                    unsigned int h );
        ~image_rsrc();
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


