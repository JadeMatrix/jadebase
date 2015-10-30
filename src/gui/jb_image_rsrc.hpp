#ifndef JADEBASE_IMAGE_RSRC_HPP
#define JADEBASE_IMAGE_RSRC_HPP

/* 
 * jb_image_rsrc.hpp
 * 
 * See jb_texture.hpp for an explanation of the filename string passed to
 * image_rsrc's constructor.
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
                    dpi::points,                                                // X position on image in points
                    dpi::points,                                                // Y position on image in points
                    dpi::points,                                                // Width on image in points
                    dpi::points );                                              // Height on image in points
        ~image_rsrc();
        
        void draw( window* );
        
    protected:
        gui_texture* texture;
        
        GLuint gl_dlist;
        bool dlist_created;
        
        dpi::points tex_pos[ 2 ];
        
        void createDisplayList();
        
        void drawTempGraphic();
    };
}

/******************************************************************************//******************************************************************************/

#endif


