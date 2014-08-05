#ifndef BQT_PLATFORM_H
#define BQT_PLATFORM_H

/* 
 * bqt_platform.h
 * 
 * Single location for (most) platform-specific stuff
 * 
 * bqt_platform_thread_t
 * bqt_platform_mutex_t
 * bqt_platform_condition_t
 * ...
 * 
 * bqt_platform_window_t
 * Type wrapper for platform window handle & OpenGL context if necessary
 * 
 * bqt_platform_idevid_t
 * Type wrapper for platform input device IDs
 * 
 * bqt_platform_keycode_t
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
    
    /* X Window System + POSIX w/ GNU extensions ******************************//******************************************************************************/
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    #include <pthread.h>
    
    typedef struct
    {
        pthread_t pt_thread;
        pthread_attr_t pt_attr;
    } bqt_platform_thread_t;
    
    typedef struct
    {
        pthread_mutex_t pt_mutex;
        pthread_mutexattr_t pt_attr;
    } bqt_platform_mutex_t;
    
    typedef struct 
    {
        pthread_cond_t pt_cond;
    } bqt_platform_condition_t;
    
    #include <X11/X.h>
    #include <GL/glxew.h>
    #include <X11/extensions/XInput.h>
    
    #define X_PROTOCOL_COUNT 1
    typedef struct
    {
        bool                     good;
        Window                 x_window;
        XWindowAttributes      x_window_attr;
        GLXContext           glx_context;
        XVisualInfo*           x_visual_info;
        XSetWindowAttributes   x_set_window_attr;
        GLint                glx_attr[ 5 ];
        Atom                   x_protocols[ X_PROTOCOL_COUNT ];
    } bqt_platform_window_t;
    
    Display* getXDisplay();
    
    typedef XID bqt_platform_idevid_t;
    
    typedef unsigned int bqt_platform_keycode_t;                                // XLib's keycode type
    
    /* Microsoft Windows ******************************************************//******************************************************************************/
    
    #elif defined PLATFORM_WINDOWS
    
    #error "Windows not implemented as a platform yet"
    
    /* Apple Mac OS X *********************************************************//******************************************************************************/
    
    #elif defined PLATFORM_MACOSX
    
    #error "Mac OS X not fully implemented as a platform yet"
    
    #endif
    
    void setQuitFlag();
    bool getQuitFlag();

    // typedef struct
    // {
    //     SDL_Window* sdl_window;
    //     SDL_GLContext sdl_gl_context;
    // } bqt_platform_window_t;

    // typedef unsigned int bqt_platform_idevid_t;

    // typedef SDL_Keycode bqt_platform_keycode_t;

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


