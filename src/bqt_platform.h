#ifndef BQT_PLATFORM_H
#define BQT_PLATFORM_H

/* 
 * bqt_platform.h
 * 
 * Single location for (most) platform-specific stuff
 * 
 * bqt_platform_window_t
 * Type wrapper for platform window handle & OpenGL context if necessary
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <SDL2/SDL.h>

/******************************************************************************//******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    SDL_Window* sdl_window;
    SDL_GLContext sdl_gl_context;
} bqt_platform_window_t;

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


