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
    public:
        image_rsrc( std::string,                                                // Image file name
                    unsigned int,                                                // X position on image in pixels
                    unsigned int,                                                // Y position on image in pixels
                    unsigned int,                                                // Width on image in pixels
                    unsigned int );                                              // Height on image in pixels
        ~image_rsrc();
        
        void draw( window* );
        
    protected:
        gui_texture* texture;
        
        GLuint gl_dlist;
        bool dlist_created;
        
        unsigned int tex_pos[ 2 ];
        
        void createDisplayList();
        
        void drawTempGraphic();
    };
}

/******************************************************************************//******************************************************************************/

#endif


