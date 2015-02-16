#ifndef JADEBASE_TEXTURE_HPP
#define JADEBASE_TEXTURE_HPP

/* 
 * jb_texture.hpp
 * 
 * Simple wrapper for OpenGL textures
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "../jb_gl.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    struct gui_texture
    {
        GLuint gl_texture;
        unsigned int dimensions[ 2 ];
        
        gui_texture()
        {
            gl_texture = 0x00;
            
            dimensions[ 0 ] = 0;
            dimensions[ 1 ] = 0;
        }
    };
    
    gui_texture* acquireTexture( std::string filename );
    void releaseTexture( gui_texture* t );
}

/******************************************************************************//******************************************************************************/

#endif


