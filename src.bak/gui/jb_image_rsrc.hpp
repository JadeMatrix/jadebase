#ifndef JADEBASE_IMAGE_RSRC_HPP
#define JADEBASE_IMAGE_RSRC_HPP

/* 
 * jb_image_rsrc.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_resource.hpp"
#include "jb_texture.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class image_rsrc : public gui_resource
    {
    protected:
        gui_texture* texture;
        
        GLuint gl_dlist;
        bool dlist_created;
        
        unsigned int tex_pos[ 2 ];
        
        void createDisplayList();
        
        void drawTempGraphic();
    public:
        image_rsrc( std::string f,
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


