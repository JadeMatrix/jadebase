#ifndef BQT_GL_HPP
#define BQT_GL_HPP

/* 
 * bqt_gl.hpp
 * 
 * OpenGL headers & utilities
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#ifdef __APPLE__
#include <OpenGL/gl.h>
// #include <OpenGL/glu.h>
#else
#include <GL/gl.h>
// #include <GL/glu.h>
#endif

/******************************************************************************//******************************************************************************/

// TODO: Remove these if they're not needed any more:
#define ZVAL_BACKGROUND -2.0f
#define ZVAL_NONCOMPSLC -1.0f
#define ZVAL_COMPDSLICE  0.0f
#define ZVAL_GUIOVERLAY  1.0f

namespace bqt
{
    void initOpenGL();                                                          // Any OpenGL initialization needed (mostly set some thread mask TASK_GPU)
}

/******************************************************************************//******************************************************************************/

#endif


