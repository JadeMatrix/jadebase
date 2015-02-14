#ifndef JADEBASE_GL_HPP
#define JADEBASE_GL_HPP

/* 
 * jb_gl.hpp
 * 
 * OpenGL headers & utilities
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_platform.h"                                                        // Includes appropriate OpenGL headers for the platform

/******************************************************************************//******************************************************************************/

namespace jade
{
    void initOpenGL();                                                          // Any OpenGL initialization needed (mostly set some thread mask TASK_GPU)
    
    GLuint bytesToTexture( unsigned char* data,
                           unsigned int w,
                           unsigned int h,
                           GLuint original = 0x00 );                            // Takes w * h bytes in data & converts it to an OpenGL texture, replacing
                                                                                // original if original != 0x00; either way it returns the texture ID.
    
    // TODO: Push & pop masks instead
    void addDrawMask( int x, int y, unsigned int w, unsigned int h );
    void clearDrawMasks();
}

/******************************************************************************//******************************************************************************/

#endif


