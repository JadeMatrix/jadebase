/* 
 * bqt_window.cpp
 * 
 * Implements bqt_window.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_window.hpp"

#include <cmath>

#include "bqt_exception.hpp"
#include "bqt_log.hpp"
#include "bqt_windowmanagement.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_preferences.hpp"
#include "bqt_launchargs.hpp"
#include "bqt_gl.hpp"

/******************************************************************************//******************************************************************************/

void drawCircle( unsigned int sides, float radius = 1.0, bool fill = true )
{
    ff::write( bqt_out, "Drawing a circle\n" );
    
    if( fill )
        glBegin( GL_POLYGON );
    else
        glBegin( GL_LINE_LOOP );
    {
        glPushMatrix();
        {
            // glTranslatef( radius, 0.0, 0.0 );
            
            for( int i = 0; i < sides; i++ )
            {
                glRotatef( 360.0f / sides, 1.0, 0.0, 0.0 );
                
                glVertex2f( radius, 0.0 );
            }
        }
        glPopMatrix();
    }
    glEnd();
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // WINDOW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        scoped_lock slock( window_mutex );
        
        #if defined PLATFORM_XWS_GNUPOSIX
        
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
        
        platform_window.glx_context = glXCreateContext( x_display,
                                                        platform_window.x_visual_info,
                                                        NULL,
                                                        GL_TRUE );
        glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
        
        platform_window.good = true;
        
        // TODO: remove
        GLenum err = glewInit();
        if( err != GLEW_OK )
        {
            bqt::exception e;
            ff::write( *e, "Failed to initialize GLEW: ", std::string( ( const char* )glewGetErrorString( err ) ) );
            throw e;
        }
        
        registerWindow( *this );
    }
    
    window::window()
    {
        platform_window.good = false;
        
        platform_window.glx_attr[ 0 ] = GLX_RGBA;
        platform_window.glx_attr[ 1 ] = GLX_DEPTH_SIZE;
        platform_window.glx_attr[ 2 ] = 24;
        platform_window.glx_attr[ 3 ] = GLX_DOUBLEBUFFER;
        platform_window.glx_attr[ 4 ] = None;
        
        // platform_window.sdl_window = NULL;
        
        pending_redraws = 0;
        
        title = BQT_WINDOW_DEFAULT_NAME;
        
        dimensions[ 0 ] = BQT_WINDOW_DEFAULT_WIDTH;
        dimensions[ 1 ] = BQT_WINDOW_DEFAULT_HEIGHT;
        position[ 0 ] = 0;
        position[ 1 ] = 0;
        
        fullscreen = false;
        in_focus = true;
        
        updates.changed    = false;
        updates.close      = false;
        updates.dimensions = false;
        updates.position   = false;
        updates.fullscreen = false;
        updates.title      = false;
        updates.minimize   = false;
        updates.maximize   = false;
        updates.restore    = false;
        
        // Drawing testing
        init_gl = true;
        init_canvas = true;
    }
    window::~window()
    {
        #if defined PLATFORM_XWS_GNUPOSIX
        
        if( platform_window.good )
        {
            Display* x_display = getXDisplay();
            
            glXMakeCurrent( x_display, None, NULL );
            glXDestroyContext( x_display, platform_window.glx_context );
            XDestroyWindow( x_display, platform_window.x_window );
            
            platform_window.good = false;
        }
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
    }
    
    void window::addCanvas( canvas* c, view_id v, int t )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::addCanvas(): Not implemented" );
    }
    void window::removeCanvas( canvas* c )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::removeCanvas(): Not implemented" );
    }
    
    void window::setToolVisibility( bool v )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::setToolVisibility(): Not implemented" );
    }
    void window::setViewZoom( view_id v, float z )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::setViewZoom(): Not implemented" );
    }
    
    std::pair< unsigned int, unsigned int > window::getDimensions()
    {
        scoped_lock slock( window_mutex );
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< int, int > window::getPosition()
    {
        scoped_lock slock( window_mutex );
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    
    float window::getViewZoom( view_id v )
    {
        scoped_lock slock( window_mutex );
        return view_zoom;
    }
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock slock( window_mutex );
        
        switch( e.id )
        {
        case STROKE:
            // // ff::write( bqt_out, e.stroke.position[ 0 ],
            // //            " ", e.stroke.position[ 1 ],
            // //            " ", e.stroke.pressure, "\n" );
            // if( e.stroke.pressure > 0 )
            {
                e.stroke.position[ 0 ] -= position[ 0 ];
                e.stroke.position[ 1 ] -= position[ 1 ];
                pending_points.push_back( e.stroke );
                submitTask( new window::redraw( *this ) );
            }
            break;
        case DROP:
            break;
        case KEYCOMMAND:
            // if( pending_points.size() )
            {
                pending_points.clear();
                init_canvas = true;
                submitTask( new window::redraw( *this ) );
            }
            break;
        case COMMAND:
            break;
        case TEXT:
            break;
        case PINCH:
            break;
        default:
            throw exception( "window::acceptEvent(): Unknown window event id" );
            break;
        }
    }
    
    bqt_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock slock( window_mutex );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    // WINDOW::MANIPULATE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::manipulate::manipulate( window* t )
    {
        if( t == NULL )
            target = new window();                                              // Useful for debugging, or if we just need a window no matter what dammit
        else
            target = t;
    }
    
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
            deregisterWindow( *target );
            target -> window_mutex.unlock();
            delete target;
            
            if( getQuitOnNoWindows() && getRegisteredWindowCount() < 1 )
            {
                if( getDevMode() )
                    ff::write( bqt_out, "All windows closed, quitting\n" );
                
                setQuitFlag();
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
                    unsigned int new_dimensions[ 2 ];
                    new_dimensions[ 0 ] = target -> dimensions[ 0 ];
                    new_dimensions[ 1 ] = target -> dimensions[ 1 ];
                    
                    if( target -> dimensions[ 0 ] < BQT_WINDOW_MIN_WIDTH )
                    {
                        new_dimensions[ 0 ] = BQT_WINDOW_MIN_WIDTH;
                        retry = true;
                    }
                    if( target -> dimensions[ 1 ] < BQT_WINDOW_MIN_HEIGHT )
                    {
                        new_dimensions[ 1 ] = BQT_WINDOW_MIN_HEIGHT;
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
                    else
                        redraw_window = true;
                    
                    target -> updates.dimensions = false;
                }
                
                if( target -> updates.active )
                {
                    target -> updates.restore = true;
                    
                    makeWindowActive( target -> getPlatformWindow() );
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
                    ff::write( bqt_out, "window::manipulate::execute(): Fullscreen not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning "window::manipulate::execute(): Fullscreen not implemented"
                    
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
                    ff::write( bqt_out, "window::manipulate::execute(): Centering not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning "window::manipulate::execute(): Centering not implemented"
                    
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
                    ff::write( bqt_out, "window::manipulate::execute(): Maximize not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning "window::manipulate::execute(): Maximize not implemented"
                    
                    makeWindowActive( target -> getPlatformWindow() );
                    
                    target -> updates.maximize = false;
                    redraw_window = true;
                }
                
                if( target -> updates.restore )
                {
                    XMapWindow( x_display,
                                target -> platform_window.x_window );
                    
                    makeWindowActive( target -> getPlatformWindow() );
                    
                    target -> updates.restore = false;
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
    
    void window::manipulate::setDimensions( unsigned int w, unsigned int h )
    {
        if( w < 1 || h < 1 )
            throw exception( "window::manipulate::setDimensions(): Width or height < 1" );
        
        scoped_lock slock( target -> window_mutex );
        
        target -> dimensions[ 0 ] = w;
        target -> dimensions[ 1 ] = h;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( int x, int y )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> position[ 0 ] = x;
        target -> position[ 1 ] = y;
        target -> updates.position = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::setFullscreen( bool f )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> fullscreen = true;
        target -> updates.fullscreen = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setTitle( std::string t )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> title = t;
        target -> updates.title = true;
        
        target -> updates.changed = true;
    }
    
    // void window::manipulate::setFocus( bool f )
    // {
    //     scoped_lock slock( target -> window_mutex );
        
    //     target -> in_focus = true;
    // }
    void window::manipulate::makeActive()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.active = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::center()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.center = true;                                        // Don't calculate here, as that code may be thread-dependent
        
        target -> updates.changed = true;
    }
    void window::manipulate::minimize()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.minimize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::maximize()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.maximize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::restore()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.restore = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::close()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.close = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::redraw()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::dropCanvas( canvas* c, unsigned int x, unsigned int y )
    {
        scoped_lock slock( target -> window_mutex );
        // TODO: implement
        throw exception( "window::manipulate::dropCanvas(): Not implemented" );
    }
    
    // WINDOW::REDRAW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock slock( target.window_mutex );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        ff::write( bqt_out, "redrawing a window\n" );
        
        scoped_lock slock( target.window_mutex );
        
        if( target.pending_redraws <= 1 )                                       // Only redraw if there are no other pending redraws for that window; this is
                                                                                // safe because the redraw task is high-priority, so the task system will
                                                                                // eventually drill down to the last one.
        {
            if( target.pending_redraws != 1 )                                   // Sanity check
                throw exception( "window::redraw::execute(): Target pending redraws somehow < 1" );
            
            // ff::write( bqt_out, "Redrawing window\n" );
            
            Display* x_display = getXDisplay();
            glXMakeCurrent( x_display,
                            target.platform_window.x_window,
                            target.platform_window.glx_context );
            
            XFlush( x_display );
            
            // {   // FBO
            //     if( target.init_gl )
            //     {
            //         glEnable( GL_TEXTURE_2D );
                    
            //         glGenFramebuffers( 1, &target.text_fbo );
            //         glGenTextures( 1, &target.texture );
            //         glGenRenderbuffers( 1, &target.render_buff );
                    
            //         if( target.text_fbo == 0x00 )
            //             throw exception( "window::redraw::execute(): Could not create FBO" );
            //         if( target.texture == 0x00 )
            //             throw exception( "window::redraw::execute(): Could not generate texture" );
                    
            //         glBindTexture( GL_TEXTURE_2D, target.texture );
            //         {
            //             glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
            //             glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            //             glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            //         }
            //         glBindTexture( GL_TEXTURE_2D, 0x00 );
                    
            //         glBindFramebuffer( GL_FRAMEBUFFER, target.text_fbo );
            //         glFramebufferTexture2D( GL_FRAMEBUFFER,
            //                                 GL_COLOR_ATTACHMENT0,
            //                                 GL_TEXTURE_2D,
            //                                 target.texture,
            //                                 0 );
                    
            //         glBindRenderbuffer( GL_RENDERBUFFER, target.render_buff );
            //         glRenderbufferStorage( GL_RENDERBUFFER,
            //                                GL_DEPTH_COMPONENT24,
            //                                256,
            //                                256 );
            //         glFramebufferRenderbuffer( GL_FRAMEBUFFER,
            //                                    GL_DEPTH_ATTACHMENT,
            //                                    GL_RENDERBUFFER,
            //                                    target.render_buff );
                    
            //         target.init_gl = false;
            //     }
                
            //     glBindFramebuffer( GL_FRAMEBUFFER, target.text_fbo );
            //     glBindRenderbuffer( GL_RENDERBUFFER, target.render_buff );
                
            //     {
            //         static float pulse = 0.0;
            //         static float inc = 0.05;
                    
            //         // ff::write( bqt_out, "Clearing texture ", target.texture, " to ", pulse, "\n" );
                    
            //         pulse += inc;
            //         if( pulse > 1.05 || pulse < 0.05 )
            //             inc *= -1;
                    
            //         glEnable(GL_DEPTH_TEST);
            //         // glViewport( 0, 0, 256, 256 );
            //         glLoadIdentity();
            //         // glOrtho( 0.0, 256, 256, 0.0, 1.0, -1.0 );
                    
            //         glClearColor( pulse, 0.5f, 0.5f, 1.0f );
            //         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                    
            //         glBegin( GL_QUADS );
            //         {
            //             glColor4f( 0.1, 0.9, 0.5, 1.0 );
            //             glVertex2f( 0.0, 0.50 );
            //             glVertex2f( 0.50, 0.0 );
            //             glVertex2f( 0.50, 0.50 );
            //             glVertex2f( 0.0, 0.0 );
            //         }
            //         glEnd();
                    
            //         // ...
            //     }
                
            //     glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            //     glBindRenderbuffer( GL_RENDERBUFFER, 0 );
            // }
            
            glXSwapBuffers( x_display, target.platform_window.x_window );
            {   // SCREEN
                // glEnable(GL_DEPTH_TEST);
                glViewport( 0, 0, target.dimensions[ 0 ], target.dimensions[ 1 ] );
                glLoadIdentity();
                glOrtho( 0.0, target.dimensions[ 0 ], target.dimensions[ 1 ], 0.0, 1.0, -1.0 );
                
                glEnable( GL_BLEND );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                
                if( target.init_canvas )
                {
                    glClearColor( 1.0, 1.0, 1.0, 1.0 );
                    glClear( GL_COLOR_BUFFER_BIT );
                    // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                    target.init_canvas = false;
                    // ff::write( bqt_out, "Cleared canvas\n" );
                }
                
                // glBegin( GL_LINE_STRIP );
                for( int i = 0; i < target.pending_points.size(); ++i )
                {
                    static bool first = true;
                    static float previous[ 3 ];
                    float width = 15.0;
                    previous[ 2 ] = 0.0;
                    
                    if( target.pending_points[ i ].pressure <= 0.0 )
                        first  = true;
                    else
                    {
                        int window_x = target.pending_points[ i ].position[ 0 ];
                        int window_y = target.pending_points[ i ].position[ 1 ];
                        
                        // ff::write( bqt_out, "Drawing point @ ", window_x, ",", window_y, " w=", width, "\n" );
                        
                        if( target.pending_points[ i ].click == CLICK_ERASE )
                        {
                            // ff::write( bqt_out, "eraser\n" );
                            glColor4f( 1.0, 1.0, 1.0, 1.0 );
                            // glColor4f( 0.0, 0.0, 0.0, 0.0 );
                        }
                        else
                            glColor4f( 0.0, 0.0, 0.0, 1.0 );
                            // glColor4f( 0.0, 0.0, 0.0, target.pending_points[ i ].pressure );
                        
                        // glVertex2f( window_x, window_y );
                        
                        drawCircle( 6, target.pending_points[ i ].pressure * width, false );
                        
                        // glPointSize( target.pending_points[ i ].pressure * width );
                        // glBegin( GL_POINTS );
                        // {
                        //     if( first )
                        //     {
                        //         glVertex2f( window_x, window_y );
                        //         previous[ 0 ] = window_x;
                        //         previous[ 1 ] = window_y;
                        //         previous[ 2 ] = target.pending_points[ i ].pressure * width;
                        //         first = false;
                        //     }
                        //     else
                        //     {
                        //         float dx = ( window_x - previous[ 0 ] );
                        //         float dy = ( window_y - previous[ 1 ] );
                                
                        //         int steps = ( int )abs( ceil( abs( dx ) > abs( dy ) ? dx : dy ) );
                                
                        //         dx /= steps;
                        //         dy /= steps;
                                
                        //         for( int j = 0; j < abs( steps ); ++j )
                        //         {
                        //             glVertex2f( previous[ 0 ] + dx * j, previous[1  ] + dy * j );
                        //         }
                                
                        //         previous[ 0 ] = window_x;
                        //         previous[ 1 ] = window_y;
                        //     }
                        // }
                        // glEnd();
                    }
                }
                // glEnd();
                
                target.pending_points.clear();
                
                
            }
            glXSwapBuffers( x_display, target.platform_window.x_window );
        }
        
        target.pending_redraws--;
        
        return true;
    }
}


