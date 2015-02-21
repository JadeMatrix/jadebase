#ifndef JADEBASE_PLATFORM_H
#define JADEBASE_PLATFORM_H

/* 
 * jb_platform.h
 * 
 * Single location for (most) platform-specific stuff
 * 
 * jb_platform_thread_t
 * jb_platform_mutex_t
 * jb_platform_condition_t
 * ...
 * 
 * jb_platform_window_t
 * Type wrapper for platform window handle & OpenGL context if necessary
 * 
 * jb_platform_idevid_t
 * Type wrapper for platform input device IDs
 * 
 * jb_platform_keycode_t
 * Type wrapper for platform keyboard key IDs
 * 
 * 
 * Platform defs:
 * 
 * PLATFORM_XWS_GNUPOSIX
 * X Window System + POSIX w/ GNU extensions - ie typical Linux distro
 * 
 * PLATFORM_WINDOWS
 * Microsoft Windows
 * 
 * PLATFORM_MACOSX
 * Apple Mac OS X
 * 
 */

/******************************************************************************//******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
    
// Platform Settings ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
/* X Window System + POSIX w/ GNU extensions **********************************//******************************************************************************/
    
    #if defined USE_DMX && !defined PLATFORM_XWS_GNUPOSIX
    #error DMX only usable with XWS
    #endif
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    #include <string.h>                                                         // For strerror_r()
    
    #define PLATFORM_FRAMEWORK_PTHREADS
    #define PLATFORM_FRAMEWORK_XLIB                                             // This platform uses GLEW, but Xlib requires a special version (GLXEW)
    
/* Microsoft Windows **********************************************************//******************************************************************************/
    
    #elif defined PLATFORM_WINDOWS
    
    #error "Windows not implemented as a platform yet"
    
/* Apple Mac OS X *************************************************************//******************************************************************************/
    
    #elif defined PLATFORM_MACOSX
    
    #define PLATFORM_FRAMEWORK_PTHREADS
    
    #include <OpenGL/glew.h>
    
    #include <string.h>                                                         // For strerror_r()
    
    // TODO: consider using Core Text instead of Pango on OS X
    
    #error "Mac OS X not fully implemented as a platform yet"
    
    #endif
    
// Framework Includes, Definitions, & Types ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
/* Pthreads *******************************************************************//******************************************************************************/
    
    #ifdef PLATFORM_FRAMEWORK_PTHREADS
    
    #include <pthread.h>
    
    typedef struct
    {
        pthread_t pt_thread;
        pthread_attr_t pt_attr;
    } jb_platform_thread_t;
    
    typedef struct
    {
        pthread_mutex_t pt_mutex;
        pthread_mutexattr_t pt_attr;
    } jb_platform_mutex_t;
    
    typedef struct 
    {
        pthread_cond_t pt_cond;
    } jb_platform_condition_t;
    
    #endif
    
/* Xlib ***********************************************************************//******************************************************************************/
    
    #ifdef PLATFORM_FRAMEWORK_XLIB
    
    #include <X11/X.h>
    #include <GL/glxew.h>
    #include <X11/extensions/XInput.h>
    #ifdef USE_DMX
    #include <X11/extensions/dmxext.h>
    #endif
    
    #define X_PROTOCOL_COUNT 1                                                  // We only need one for now to catch window closes from the WM
    typedef struct
    {
        int                      good;                                          // 'bool'
        Window                 x_window;
        XWindowAttributes      x_window_attr;
        GLXContext           glx_context;
        XVisualInfo*           x_visual_info;
        XSetWindowAttributes   x_set_window_attr;
        GLint                glx_attr[ 5 ];
        Atom                   x_protocols[ X_PROTOCOL_COUNT ];
    } jb_platform_window_t;
    
    Display* getXDisplay();
    
    typedef XID jb_platform_idevid_t;
    
    typedef unsigned int jb_platform_keycode_t;                                 // XLib's keycode type
    
    #endif
    
// General / Other /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // TODO: Move to a different header?
    
    void jb_setQuitFlag();
    int jb_getQuitFlag();                                                       // 'bool'

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


