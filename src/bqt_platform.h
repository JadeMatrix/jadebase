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
    
    #if defined USE_DMX && !defined PLATFORM_XWS_GNUPOSIX
    #error DMX only usable with XWS
    #endif
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    #include <string.h>                                                         // For strerror_r()
    
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
    } bqt_platform_window_t;
    
    Display* getXDisplay();
    
    typedef struct
    {
        int x_real_dev;                                                         // 'bool': We want to be able to associate a dummy ID with the core pointer
        union
        {
            unsigned int bqt_devid;
            XID x_devid;
        };
    } bqt_platform_idevid_t;
    
    int bqt_platform_idevid_t_comp( const bqt_platform_idevid_t left,           // Returns 'bool'
                                    const bqt_platform_idevid_t right );
    typedef int ( * bqt_platform_idevid_t_comp_t )( const bqt_platform_idevid_t,
                                                    const bqt_platform_idevid_t );
    
    typedef unsigned int bqt_platform_keycode_t;                                // XLib's keycode type
    
    /* Microsoft Windows ******************************************************//******************************************************************************/
    
    #elif defined PLATFORM_WINDOWS
    
    #error "Windows not implemented as a platform yet"
    
    /* Apple Mac OS X *********************************************************//******************************************************************************/
    
    #elif defined PLATFORM_MACOSX
    
    #include <string.h>                                                         // For strerror_r()
    
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
    
    typedef struct
    {
        pthread_rwlock_t pt_rwlock;
        pthread_rwlockattr_t pt_attr;
    } bqt_platform_rwlock_t;
    
    #include <OpenGL/glew.h>
    
    // TODO: consider using Core Text instead of Pango on OS X
    
    #error "Mac OS X not fully implemented as a platform yet"
    
    #endif
    
    /* General ****************************************************************//******************************************************************************/
    
    // TODO: Move to a different header?
    
    void setQuitFlag();
    int getQuitFlag();                                                          // 'bool'

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


