#ifndef JADEBASE_IMAGE_RSRC_HPP
#define JADEBASE_IMAGE_RSRC_HPP

/* 
 * jb_image_rsrc.hpp
 * 
 * The string passed to image_rsrc's constructor is the 1x zoom version.  If the
 * 1x zoom version is named "image.png" then any file in the same directory
 * named "image@2x.png" will also be loaded as the hi-DPI (2x zoom) version.
 * If the string ends in "@2x.png", that file will be loaded as the 2x zoom
 * version; if file with the "@2x" removed exists it will be loaded as the 1x
 * zoom version.  Currently only PNG files following this Apple naming scheme
 * are supported.  The position and dimensions passed into the constructor have
 * a ratio of 1 point per pixel on the 1x zoom version.
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


