#ifndef JADEBASE_TEXTURE_HPP
#define JADEBASE_TEXTURE_HPP

/* 
 * jb_texture.hpp
 * 
 * Simple wrapper for managed OpenGL textures; implemented in
 * jb_named_resources.cpp
 * 
 * The string passed to acquireTexture() by default is the 1x zoom version.  If
 * the 1x zoom version is named "image.png" then any file in the same directory
 * named "image@2x.png" will also be loaded as the hi-DPI (2x zoom) version.  If
 * the supplied string ends in "@2x.png", that file will be loaded as the 2x
 * zoom version; if file without the "@2x" exists it will be loaded as the 1x
 * zoom version.  Currently only PNG files following this Apple-style naming
 * scheme are supported.  The position and dimensions passed into the construc-
 * tor have a ratio of 1 point per pixel on the 1x zoom version.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "../utility/jb_dpi.hpp"
#include "../utility/jb_gl.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    struct gui_texture
    {
        GLuint gl_texture;
        dpi::points dimensions[ 2 ];
        
        gui_texture()
        {
            gl_texture = 0x00;
            
            dimensions[ 0 ] = 0;
            dimensions[ 1 ] = 0;
        }
    };
    
    gui_texture* acquireTexture( std::string );                                 // Lock & return (possibly opening) a texture from the image at the given path
    gui_texture* acquireTexture( dpi::points,                                   // Create a managed texture from a raw data descriptor
                                 dpi::points,                                   // Width & height in whole points
                                 unsigned char*[ 2 ] );                         // Two mipmap levels; 0 = 1x, 1 = 2x; either, neither, but not both may be NULL;
                                                                                // the texture management system owns these pointers after passing them in.
    gui_texture* acquireTexture( gui_texture* );                                // Reference an existing managed texture
    void releaseTexture( gui_texture* t );                                      // Release an acquired texture
}

/******************************************************************************//******************************************************************************/

#endif


