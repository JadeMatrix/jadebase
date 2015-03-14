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
    
    GLuint bytesToTexture( unsigned char*,                                      // Pixel data in RGBA_8
                           unsigned int,                                        // Width in pixels
                           unsigned int,                                        // Height in pixels
                           GLuint original = 0x00 );                            // Takes w * h bytes in data & converts it to an OpenGL texture, replacing
                                                                                // original if original != 0x00; either way it returns the texture ID.
    
    // TODO: Push & pop masks instead
    void addDrawMask( int,                                                      // Pixel position x on the view to overlay the mask
                      int,                                                      // Pixel position y on the view to overlay the mask
                      unsigned int,                                             // Pixel width of the mask
                      unsigned int );                                           // Pixel heigth of the masks
                                                                                // Draw masks prevent OpenGL drawing outside of the area describe, but can be
                                                                                // ORed together to create non-rectangular areas (for now).
    void clearDrawMasks();                                                      // Clears all current draw masks
}

/******************************************************************************//******************************************************************************/

#endif


