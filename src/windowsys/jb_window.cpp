/* 
 * jb_window.cpp
 * 
 * Implements jb_window.hpp
 * 
 */

// http://www.opengl.org/wiki/Texture_Storage#Texture_copy

// http://tronche.com/gui/x/xlib/pixmap-and-cursor/cursor.html
// http://tronche.com/gui/x/xlib/window/XDefineCursor.html

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

#include <cmath>

#include "jb_windowmanagement.hpp"
#include "../gui/jb_named_resources.hpp"
#include "../tasking/jb_taskexec.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"

// testing
#include "../gui/jb_button.hpp"
#include "../gui/jb_dial.hpp"
#include "../gui/jb_tabset.hpp"
#include "../gui/jb_group.hpp"
#include "../gui/jb_scrollset.hpp"
#include "../gui/jb_image_rsrc.hpp"
#include "../gui/jb_text_rsrc.hpp"
#include "../gui/jb_canvasview.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // WINDOW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        scoped_lock< mutex > slock( window_mutex );
        
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
            
            glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
            
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
        
        jade::initNamedResources();                                             // These will be deinitialized when quitting
        
        top_group = new group( *this, 0, 0, dimensions[ 0 ], dimensions[ 1 ] ); // Top-level group that holds all GUI elements
        top_group -> drawBackground( false );
    }
    
    void window::makeContextCurrent()
    {
        Display* x_display = getXDisplay();
        glXMakeCurrent( x_display,
                        platform_window.x_window,
                        platform_window.glx_context );
        
        XFlush( x_display );
    }
    
    window::~window()
    {
        scoped_lock< mutex > slock( window_mutex );                             // If we really need this we have bigger problems (pending tasks, etc.)
        
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
    
    window::window()
    {
        platform_window.good = false;
        
        platform_window.glx_attr[ 0 ] = GLX_RGBA;
        platform_window.glx_attr[ 1 ] = GLX_DEPTH_SIZE;
        platform_window.glx_attr[ 2 ] = 24;
        platform_window.glx_attr[ 3 ] = GLX_DOUBLEBUFFER;
        platform_window.glx_attr[ 4 ] = None;
        
        pending_redraws = 0;
        
        title = JADEBASE_WINDOW_DEFAULT_NAME;
        
        std::pair< unsigned int, unsigned int > def_dim( getSetting_num( "jb_DefaultWindowW" ),
                                                         getSetting_num( "jb_DefaultWindowH" ) );
        
        dimensions[ 0 ] = def_dim.first;
        dimensions[ 1 ] = def_dim.second;
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
        updates.redraw     = false;
    }
    
    std::pair< unsigned int, unsigned int > window::getDimensions()
    {
        scoped_lock< mutex > slock( window_mutex );
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< int, int > window::getPosition()
    {
        scoped_lock< mutex > slock( window_mutex );
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( window_mutex );
        
////////// Devel  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if( e.type == KEYCOMMAND && e.key.key == KEY_Q && e.key.cmd && e.key.up )
        {
            jb_setQuitFlag();
            return;
        }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        bool no_position = false;
        
        int e_position[ 2 ];
        
        e.offset[ 0 ] = 0;
        e.offset[ 1 ] = 0;
        
        switch( e.type )                                                        // Normalize event position to window position (if necessary)
        {
        case STROKE:
            e.stroke.position[ 0 ] -= position[ 0 ];
            e.stroke.position[ 1 ] -= position[ 1 ];
            e.stroke.prev_pos[ 0 ] -= position[ 0 ];
            e.stroke.prev_pos[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.stroke.position[ 0 ];
            e_position[ 1 ] = e.stroke.position[ 1 ];
            break;
        case DROP:
            e.drop.position[ 0 ] -= position[ 0 ];
            e.drop.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.drop.position[ 0 ];
            e_position[ 1 ] = e.drop.position[ 1 ];
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            no_position = true;
            break;
        case PINCH:
            e.pinch.position[ 0 ] -= position[ 0 ];
            e.pinch.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.pinch.position[ 0 ];
            e_position[ 1 ] = e.pinch.position[ 1 ];
            break;
        case SCROLL:
            e.scroll.position[ 0 ] -= position[ 0 ];
            e.scroll.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.scroll.position[ 0 ];
            e_position[ 1 ] = e.scroll.position[ 1 ];
            break;
        default:
            throw exception( "window::acceptEvent(): Unknown event type" );
            break;
        }
        
        if( e.type == STROKE
            && input_assoc.count( e.stroke.dev_id ) )                           // Check elements in device association list first
        {
            idev_assoc& assoc( input_assoc[ e.stroke.dev_id ] );
            
            e.offset[ 0 ] = assoc.offset[ 0 ];
            e.offset[ 1 ] = assoc.offset[ 1 ];
            
            assoc.element -> acceptEvent( e );
            
            return;
        }
        
        top_group -> acceptEvent( e );                                          // Send event to top-level group (at 0,0; dimensions match window)
    }
    
    jb_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    void window::associateDevice( jb_platform_idevid_t dev_id,
                                  gui_element* element,
                                  float off_x,
                                  float off_y )
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        idev_assoc& assoc( input_assoc[ dev_id ] );
        
        if( element == NULL )
            throw exception( "window::associateDevice(): Attempt to associate a device with a NULL element" );
        
        assoc.element = element;
        assoc.offset[ 0 ] = off_x;
        assoc.offset[ 1 ] = off_y;
    }
    void window::deassociateDevice( jb_platform_idevid_t dev_id )
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        if( !input_assoc.erase( dev_id ) && getDevMode() )
            ff::write( jb_out, "Warning: Attempt to deassociate a non-associated device\n" );
    }
    
    std::string window::getTitle()
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        return title;
    }
    
    group* window::getTopGroup()
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        return top_group;
    }
    
    void window::requestRedraw()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( pending_redraws < 1 )
        {
            submitTask( new redraw( *this ) );
        }
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
            /* GUI CLEANUP ****************************************************//******************************************************************************/
            
            target -> top_group -> closed();
            delete target -> top_group;
            target -> top_group = NULL;
            
            /* WINDOW CLEANUP *************************************************//******************************************************************************/
            
            deregisterWindow( *target );
            target -> window_mutex.unlock();
            delete target;
            
            if( getSetting_bln( "jb_QuitOnNoWindows" ) && getRegisteredWindowCount() < 1 )
            {
                if( getDevMode() )
                    ff::write( jb_out, "All windows closed, quitting\n" );
                
                jb_setQuitFlag();
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
                    
                    if( target -> dimensions[ 0 ] < JADEBASE_WINDOW_MIN_WIDTH )
                    {
                        new_dimensions[ 0 ] = JADEBASE_WINDOW_MIN_WIDTH;
                        retry = true;
                    }
                    if( target -> dimensions[ 1 ] < JADEBASE_WINDOW_MIN_HEIGHT )
                    {
                        new_dimensions[ 1 ] = JADEBASE_WINDOW_MIN_HEIGHT;
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
                    else                                                        // Window size change OK, so update top_group
                    {
                        target -> top_group -> setRealDimensions( target -> dimensions[ 0 ],
                                                                  target -> dimensions[ 1 ] );
                        
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
    
    void window::manipulate::setDimensions( unsigned int w, unsigned int h )
    {
        if( w < 1 || h < 1 )
            throw exception( "window::manipulate::setDimensions(): Width or height < 1" );
        
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> dimensions[ 0 ] = w;
        target -> dimensions[ 1 ] = h;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( int x, int y )
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> position[ 0 ] = x;
        target -> position[ 1 ] = y;
        target -> updates.position = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::setFullscreen( bool f )
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> fullscreen = true;
        target -> updates.fullscreen = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setTitle( std::string t )
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> title = t;
        target -> updates.title = true;
        
        target -> updates.changed = true;
    }
    
    // void window::manipulate::setFocus( bool f )
    // {
    //     scoped_lock< mutex > slock( target -> window_mutex );
        
    //     target -> in_focus = true;
    // }
    
    void window::manipulate::center()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.center = true;                                        // Don't calculate here, as that code may be thread-dependent
        
        target -> updates.changed = true;
    }
    void window::manipulate::minimize()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.minimize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::maximize()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.maximize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::restore()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.restore = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::close()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.close = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::redraw()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.redraw = true;
        
        target -> updates.changed = true;
    }
    
    // WINDOW::REDRAW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock< mutex > slock( target.window_mutex );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        scoped_lock< mutex > slock( target.window_mutex );
        
        target.makeContextCurrent();
        
        if( target.pending_redraws <= 1 )                                       // Only redraw if there are no other pending redraws for that window; this is
                                                                                // safe because the redraw task is high-priority, so the task system will
                                                                                // eventually drill down to the last one.
        {
            if( target.pending_redraws != 1 )                                   // Sanity check
                throw exception( "window::redraw::execute(): Target pending redraws somehow < 1" );
            
            glViewport( 0, 0, target.dimensions[ 0 ], target.dimensions[ 1 ] );
            glLoadIdentity();
            glOrtho( 0.0, target.dimensions[ 0 ], target.dimensions[ 1 ], 0.0, 1.0, -1.0 );
            
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
            // glEnable( GL_DEPTH_TEST );
            glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
            glClearStencil( 0 );
            // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
            glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
            
            glEnable( GL_TEXTURE_2D );
            
            // glEnable( GL_POLYGON_SMOOTH );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            
            target.top_group -> draw();
            
            #if defined PLATFORM_XWS_GNUPOSIX
            
            Display* x_display = getXDisplay();
            glXSwapBuffers( x_display, target.platform_window.x_window );
            
            #else
            
            #error "Buffer swapping not implemented on non-X platforms"
            
            #endif
        }
        
        target.pending_redraws--;
        
        return true;
    }
}


