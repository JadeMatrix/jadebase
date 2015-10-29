#ifndef JADEBASE_TEXTURE_HPP
#define JADEBASE_TEXTURE_HPP

/* 
 * jb_texture.hpp
 * 
 * Simple wrapper for OpenGL textures
 * 
 * Implemented in jb_named_resources.cpp
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
    void releaseTexture( gui_texture* t );                                      // Release an acquired texture
}

/******************************************************************************//******************************************************************************/

#endif


