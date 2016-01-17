/* 
 * x_window.cpp
 * 
 * Platform-specific code from jb_window.hpp on X-based platforms
 * 
 */

#ifndef PLATFORM_XWS_GNUPOSIX
#error PLATFORM_XWS_GNUPOSIX not defined
#endif

// http://tronche.com/gui/x/xlib/pixmap-and-cursor/cursor.html
// http://tronche.com/gui/x/xlib/window/XDefineCursor.html

// TODO: Invesitigate how to group windows in e.g. the Ubuntu task bar, possibly XWMHINTS WindowGroupHint
// http://tronche.com/gui/x/xlib/ICC/client-to-window-manager/wm-hints.html

// Also a note about setting window icons on 32 vs. 54 bit
// http://stackoverflow.com/questions/10699927/xlib-argb-window-icon

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

#include <cmath>

#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    dpi::pixels window::getDPI()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        Display* x_display = getXDisplay();
        
        dpi::percent x_display_ppmm[ 2 ];
        
        x_display_ppmm[ 0 ] = 25.4f
                              * ( dpi::percent )DisplayWidth(   x_display, 0 )
                              / ( dpi::percent )DisplayWidthMM( x_display, 0 );
        x_display_ppmm[ 1 ] = 25.4f
                              * ( dpi::percent )DisplayHeight(   x_display, 0 )
                              / ( dpi::percent )DisplayHeightMM( x_display, 0 );
        
        return ( x_display_ppmm[ 0 ] + x_display_ppmm[ 1 ] ) / 2.0f;
    }
    dpi::percent window::getScaleFactor()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        float scale_override = getGUIScaleOverride();
        
        if( isnan( scale_override) )
            return ( dpi::percent )getDPI() / ( dpi::percent )STANDARD_DPI;
        else
            return scale_override;
    }
    
    // TODO: Potentially move back to jb_window.cpp
    jb_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    void window::platformWindowConstructor()
    {
        platform_window.good = false;
        
        platform_window.glx_attr[ 0 ] = GLX_RGBA;
        platform_window.glx_attr[ 1 ] = GLX_DEPTH_SIZE;
        platform_window.glx_attr[ 2 ] = 24;
        platform_window.glx_attr[ 3 ] = GLX_DOUBLEBUFFER;
        platform_window.glx_attr[ 4 ] = None;
    }
    void window::init()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        Display* x_display = getXDisplay();
        Window x_root = DefaultRootWindow( x_display );
        
        XMapWindow( x_display, x_root );
        
        XFlush( x_display );
        
        platform_window.x_visual_info = glXChooseVisual( x_display,
                                                         DefaultScreen( x_display ),
                                                         platform_window.glx_attr );
        
        if( platform_window.x_visual_info == NULL )
            throw exception( "window::init(): No appropriate X visual found" );
        
        // TODO: Do we need to initialize platform_window.x_set_window_att?
        platform_window.x_set_window_attr.colormap = XCreateColormap( x_display,
                                                                      x_root,
                                                                      platform_window.x_visual_info -> visual,
                                                                      AllocNone );
        platform_window.x_set_window_attr.event_mask = KeyPressMask
                                                       | KeyReleaseMask
                                                       | ButtonPressMask
                                                       | ButtonReleaseMask
                                                       | EnterWindowMask
                                                       | LeaveWindowMask
                                                       | PointerMotionMask
                                                       // | PointerMotionHintMask
                                                       // | Button1MotionMask
                                                       // | Button2MotionMask
                                                       // | Button3MotionMask
                                                       // | Button4MotionMask
                                                       // | Button5MotionMask
                                                       | ButtonMotionMask
                                                       // | KeyMapStateMask
                                                       | ExposureMask
                                                       | VisibilityChangeMask
                                                       | StructureNotifyMask
                                                       // | ResizeRedirectMask
                                                       | SubstructureNotifyMask
                                                       // | SubstructureRedirectMask
                                                       | FocusChangeMask
                                                       | PropertyChangeMask
                                                       | ColormapChangeMask
                                                       // | OwnerGrabButtonMask
                                                       ;
        
        platform_window.x_window = XCreateWindow( x_display,
                                                  x_root,
                                                  position[ 0 ],
                                                  position[ 1 ],
                                                  dimensions[ 0 ],
                                                  dimensions[ 1 ],
                                                  0,                            // Window border width, unused
                                                  platform_window.x_visual_info -> depth,
                                                  InputOutput,
                                                  platform_window.x_visual_info -> visual,
                                                  CWColormap | CWEventMask,
                                                  &platform_window.x_set_window_attr );
        
        XMapWindow( x_display, platform_window.x_window );
        XStoreName( x_display, platform_window.x_window, title.c_str() );
        
        platform_window.x_protocols[ 0 ] = XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False );
        if( !XSetWMProtocols( x_display,
                              platform_window.x_window,
                              platform_window.x_protocols,
                              X_PROTOCOL_COUNT ) )
            throw exception( "window::init(): Could not set X window protocols" );
        
        XFlush( x_display );                                                    // We need to flush X before creating the GLX Context
        
        if( !getRegisteredWindowCount() )                                       // No windows registered, so create a context
        {
            if( getDevMode() )
                ff::write( jb_out,
                           "Creating a blank GLX context for window id 0x",
                           ff::to_x( platform_window.x_window ),
                           "\n" );
            
            platform_window.glx_context = glXCreateContext( x_display,
                                                            platform_window.x_visual_info,
                                                            NULL,
                                                            GL_TRUE );
            
            initOpenGL();                                                       // Init OpenGL first time only
            
            glXMakeCurrent( x_display,
                            platform_window.x_window,
                            platform_window.glx_context );
            
            glewExperimental = GL_TRUE;                                         // To allow FBOs in OpenGL <3.0
            GLenum err = glewInit();                                            // Init GLEW first time only
                                                                                // TODO: Move to initOpenGL()?
            if( err != GLEW_OK )
            {
                jade::exception e;
                ff::write( *e, "Failed to initialize GLEW: ", std::string( ( const char* )glewGetErrorString( err ) ) );
                throw e;
            }
        }
        else
        {
            window& context_source( getAnyWindow() );
            
            if( getDevMode() )
                ff::write( jb_out,
                           "Creating a GLX context for window id 0x",
                           ff::to_x( platform_window.x_window ),
                           " from window id 0x",
                           ff::to_x( context_source.getPlatformWindow().x_window ),
                           "\n" );
            
            platform_window.glx_context = glXCreateContext( x_display,
                                                            platform_window.x_visual_info,
                                                            context_source.getPlatformWindow().glx_context,
                                                            GL_TRUE );
            
            glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
        }
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
        
        platform_window.good = true;
        
        registerWindow( *this );
        
        top_element -> setDrawBackground( false );
        top_element -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] );
    }
    
    void window::makeContextCurrent()
    {
        Display* x_display = getXDisplay();
        glXMakeCurrent( x_display,
                        platform_window.x_window,
                        platform_window.glx_context );
        
        XFlush( x_display );
    }
    void window::swapBuffers()
    {
        Display* x_display = getXDisplay();
        glXSwapBuffers( x_display, target.platform_window.x_window );
    }
    
    window::~window()
    {
        scoped_lock< mutex > slock( window_mutex );                             // If we really need this we have bigger problems (pending tasks, etc.)
        
        if( platform_window.good )
        {
            Display* x_display = getXDisplay();
            
            glXMakeCurrent( x_display, None, NULL );
            glXDestroyContext( x_display, platform_window.glx_context );
            XDestroyWindow( x_display, platform_window.x_window );
            XFree( platform_window.x_visual_info );
            
            platform_window.good = false;
        }
    }
    
    /* window::ManipulateWindow_task ******************************************//******************************************************************************/
    
    bool window::ManipulateWindow_task::windowNeedsInit()
    {
        return !( target -> platform_window.good );
    }
    
    // CHECK: These may need XFlush( x_display );
    
    void window::ManipulateWindow_task::updateDimensions()
    {
        XResizeWindow( getXDisplay(),
                       target -> platform_window.x_window,
                       dimensions[ 0 ],
                       dimensions[ 1 ] );
    }
    void window::ManipulateWindow_task::updatePosition()
    {
        XMoveWindow( getXDisplay(),
                     target -> platform_window.x_window,
                     position[ 0 ],
                     position[ 1 ] );
    }
    void window::ManipulateWindow_task::updateFullscreen()
    {
        ff::write( jb_out, "window::ManipulateWindow_task::updateFullscreen(): Fullscreen not implemented yet, ignoring\n" );
        #warning window::ManipulateWindow_task::updateFullscreen(): Fullscreen not implemented
        // IMPLEMENT:
    }
    void window::ManipulateWindow_task::updateTitle()
    {
        XStoreName( getXDisplay(),
                    target -> platform_window.x_window,
                    target -> title.c_str() );
    }
    void window::ManipulateWindow_task::updateCenter()
    {
        ff::write( jb_out, "window::ManipulateWindow_task::updateCenter(): Centering not implemented yet, ignoring\n" );
        #warning window::ManipulateWindow_task::updateCenter(): Centering not implemented
        // IMPLEMENT:
    }
    void window::ManipulateWindow_task::updateMinimize()
    {
        XIconifyWindow( getXDisplay(),
                        target -> platform_window.x_window,
                        DefaultScreen( x_display ) );
    }
    void window::ManipulateWindow_task::updateMaximize()
    {
        ff::write( jb_out, "window::ManipulateWindow_task::updateMaximize(): Maximize not implemented yet, ignoring\n" );
        #warning window::ManipulateWindow_task::updateMaximize(): Maximize not implemented
        // IMPLEMENT:
    }
    void window::ManipulateWindow_task::updateRestore()
    {
        XMapWindow( getXDisplay(),
                    target -> platform_window.x_window );
    }
}


