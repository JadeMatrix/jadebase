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

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    dpi::pixels window::getDPI()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        Display* x_display = getXDisplay();
        
        using dpi::percent;
        using dpi::pixels;
        
        percent x_display_ppmm[ 2 ];
        
        x_display_ppmm[ 0 ] = 25.4f
                              * ( percent )DisplayWidth(   x_display, 0 )
                              / ( percent )DisplayWidthMM( x_display, 0 );
        x_display_ppmm[ 1 ] = 25.4f
                              * ( percent )DisplayHeight(   x_display, 0 )
                              / ( percent )DisplayHeightMM( x_display, 0 );
        
        return ( x_display_ppmm[ 0 ] + x_display_ppmm[ 1 ] ) / 2.0f;
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
    
    /* window::manipulate *****************************************************//******************************************************************************/
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        bool redraw_window = false;
        
        target -> window_mutex.lock();                                          // We need to explicitly lock/unlock this as the window can be destroyed
        
        if( !( target -> platform_window.good ) )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( target -> updates.close )
        {
            /* CONTAINER CLEANUP **********************************************//******************************************************************************/
            
            target -> can_add_containers = false;
            
            for( std::set< container< window >* >::iterator iter = target -> containers.begin();
                 iter != target -> containers.end();
                 ++iter )
            {
                ( *iter ) -> clear();
            }
            
            /* GUI CLEANUP ****************************************************//******************************************************************************/
            
            target -> top_element -> closed();                                  // This is essentially the point where children elements think the window closes
            // Window will destroy std::shared_ptr when deleted
            
            /* WINDOW CLEANUP *************************************************//******************************************************************************/
            
            deregisterWindow( *target );
            target -> window_mutex.unlock();
            delete target;
            
            if( getRegisteredWindowCount() < 1 )
            {
                bool should_quit;
                if( !getSetting( "jb_QuitOnNoWindows", should_quit ) )
                    #ifdef PLATFORM_MACOSX
                    should_quit = false;
                    #else
                    should_quit = true;
                    #endif
                
                if( should_quit )
                {
                    if( getDevMode() )
                        ff::write( jb_out, "All windows closed, quitting\n" );
                    
                    jb_setQuitFlag();
                }
            }
        }
        else
        {
            if( target -> updates.changed )
            {
                Display* x_display = getXDisplay();
                
                if( target -> updates.dimensions )
                {
                    // Actual resizing handled by WM, this is just for bounds checking
                    bool retry = false;
                    dpi::pixels new_dimensions[ 2 ];
                    new_dimensions[ 0 ] = target -> dimensions[ 0 ];
                    new_dimensions[ 1 ] = target -> dimensions[ 1 ];
                    
                    dpi::percent scale = target -> getScaleFactor();
                    
                    // TODO: Fluid syncing of window & top_element dimensions:
                    //         1. Window gets dimension changed
                    //         2. If good, window sets top_element dimensions
                    //         3. top_element clips dimensions to calculated
                    //            mins (if any)
                    //         4. Window then gets top_element dimensions and
                    //            sets self dimensions from those
                    //         No min window dimensions (X will be happier)
                    
                    if( target -> dimensions[ 0 ] < JADEBASE_WINDOW_MIN_WIDTH * scale )
                    {
                        new_dimensions[ 0 ] = JADEBASE_WINDOW_MIN_WIDTH * scale;
                        retry = true;
                    }
                    if( target -> dimensions[ 1 ] < JADEBASE_WINDOW_MIN_HEIGHT * scale )
                    {
                        new_dimensions[ 1 ] = JADEBASE_WINDOW_MIN_HEIGHT * scale;
                        retry = true;
                    }
                    
                    if( retry )
                    {
                        XResizeWindow( x_display,
                                       target -> platform_window.x_window,
                                       new_dimensions[ 0 ],
                                       new_dimensions[ 1 ] );                   // This will generate a new event that leads here agan; this is OK
                        
                        redraw_window = false;
                    }
                    else                                                        // Window size change OK, so update top_element
                    {
                        target -> top_element -> setRealDimensions( target -> dimensions[ 0 ] / scale,
                                                                    target -> dimensions[ 1 ] / scale );
                        
                        redraw_window = true;
                    }
                    
                    target -> updates.dimensions = false;
                }
                
                if( target -> updates.position )
                {
                    // Actual moving handled by WM
                    // XMoveWindow( x_display,
                    //              target -> platform_window.x_window,
                    //              target -> position[ 0 ],
                    //              target -> position[ 1 ] );
                    
                    target -> updates.position = false;
                }
                
                if( target -> updates.fullscreen )
                {
                    ff::write( jb_out, "window::manipulate::execute(): Fullscreen not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Fullscreen not implemented
                    
                    target -> updates.fullscreen = false;
                    redraw_window = true;
                }
                
                if( target -> updates.title )
                {
                    XStoreName( x_display,
                                target -> platform_window.x_window,
                                target -> title.c_str() );
                    
                    target -> updates.title = false;
                }
                
                if( target -> updates.center )
                {
                    ff::write( jb_out, "window::manipulate::execute(): Centering not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Centering not implemented
                    
                    target -> updates.center = false;
                }
                
                if( target -> updates.minimize )
                {
                    XIconifyWindow( x_display,
                                    target -> platform_window.x_window,
                                    DefaultScreen( x_display ) );
                    
                    target -> updates.minimize = false;
                }
                
                if( target -> updates.maximize )
                {
                    ff::write( jb_out, "window::manipulate::execute(): Maximize not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Maximize not implemented
                    
                    
                    
                    target -> updates.maximize = false;
                    redraw_window = true;
                }
                
                if( target -> updates.restore )
                {
                    XMapWindow( x_display,
                                target -> platform_window.x_window );
                    
                    
                    
                    target -> updates.restore = false;
                    redraw_window = true;
                }
                
                if( target -> updates.redraw )
                {
                    target -> updates.redraw = false;
                    
                    redraw_window = true;
                }
                
                target -> updates.changed = false;
                
                XFlush( x_display );
            }
            
            target -> window_mutex.unlock();
            
            if( redraw_window )
                submitTask( new window::redraw( *target ) );
        }
        
        return true;
    }
}


