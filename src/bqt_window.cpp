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
#include "gui/bqt_named_resources.hpp"

#include "gui/bqt_gui_button.hpp"
#include "gui/bqt_gui_dial.hpp"
#include "gui/bqt_gui_tabset.hpp"
#include "gui/bqt_gui_group.hpp"
#include "gui/bqt_gui_scrollset.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
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
        
        window* active_window = getActiveWindow();
        if( active_window == NULL )
        {
            if( getDevMode() )
                ff::write( bqt_out,
                           "Creating a blank GLX context for window id 0x",
                           ff::to_x( platform_window.x_window ),
                           "\n" );
            
            platform_window.glx_context = glXCreateContext( x_display,
                                                        platform_window.x_visual_info,
                                                        NULL,
                                                        GL_TRUE );
            
            initOpenGL();                                                       // Init OpenGL first time only
            
            glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
            
            GLenum err = glewInit();                                            // Init GLEW first time only
                                                                                // TODO: Move to initOpenGL()?
            if( err != GLEW_OK )
            {
                bqt::exception e;
                ff::write( *e, "Failed to initialize GLEW: ", std::string( ( const char* )glewGetErrorString( err ) ) );
                throw e;
            }
        }
        else
        {
            if( getDevMode() )
                ff::write( bqt_out,
                           "Creating a GLX context for window id 0x",
                           ff::to_x( platform_window.x_window ),
                           " from window id 0x",
                           ff::to_x( active_window -> getPlatformWindow().x_window ),
                           "\n" );
            
            platform_window.glx_context = glXCreateContext( x_display,
                                                        platform_window.x_visual_info,
                                                        active_window -> getPlatformWindow().glx_context,
                                                        GL_TRUE );
            
            glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
        }
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
        
        platform_window.good = true;
        
        registerWindow( *this );
        
        // Devel
        {
            group* test_group_a = new group( *this, 0, 0, 256, 256 );
            group* test_group_b = new group( *this, 0, 0, 256, 256 );
            group* test_group_c = new group( *this, 0, 0, 256, 256 );
            
            test_group_a -> addElement( new button( *this, 10, 10, 60, 40 ) );
            test_group_a -> addElement( new button( *this, 72, 10, 26, 40 ) );
            test_group_a -> addElement( new button( *this, 100, 10, 40, 19 ) );
            test_group_a -> addElement( new button( *this, 100, 31, 40, 19 ) );
            
            group* scroll_group = new group( *this, 0, 0, 256, 256 );
            scroll_group -> setEventFallthrough( true );
            
            scroll_group -> addElement( new dial( *this, 10, 10 ) );
            scroll_group -> addElement( new dial( *this, 62, 10, true ) );
            
            // Groups can't set their elements' dimensions automatically, so we subtract TABSET_BAR_HEIGHT
            test_group_b -> addElement( new scrollset( *this, 0, 0, 256, 256 - TABSET_BAR_HEIGHT, scroll_group ) );
            
            tabset* test_tabset = new tabset( *this, 0, 0, 256, 256 );
            
            // test_tabset -> addTab( test_group_a, "test tab A" );
            // test_tabset -> addTab( test_group_b, "test tab B" );
            test_tabset -> addTab( test_group_a, "Open_file_A.bqt" );
            test_tabset -> addTab( test_group_b, "難しい漢字の時.bqt" );
            test_tabset -> addTab( test_group_c, "النص العربي.bqt" );
            
            elements.push_back( test_tabset );
            
            requestRedraw();
        }
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
        scoped_lock< mutex > slock( window_mutex );                             // If we really need this we have bigger problems
        
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
        
        // platform_window.sdl_window = NULL;
        
        pending_redraws = 0;
        
        title = BQT_WINDOW_DEFAULT_NAME;
        
        std::pair< unsigned int, unsigned int > def_dim = getDefaultWindowDimensions();
        
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
        
        // Devel
        if( e.type == KEYCOMMAND && e.key.key == KEY_Q && e.key.cmd && e.key.up )
        {
            setQuitFlag();
            return;
        }
        
        bool no_position = false;
        
        std::pair< int, int > element_position;
        std::pair< unsigned int, unsigned int > element_dimensions;
        int e_position[ 2 ];
        
        e.offset[ 0 ] = 0;
        e.offset[ 1 ] = 0;
        
        switch( e.type )
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
            && input_assoc.count( e.stroke.dev_id ) )
        {
            idev_assoc& assoc( input_assoc[ e.stroke.dev_id ] );
            
            e.offset[ 0 ] = assoc.offset[ 0 ];
            e.offset[ 1 ] = assoc.offset[ 1 ];
            
            assoc.element -> acceptEvent( e );
            
            return;
        }
        
        for( int i = elements.size() - 1; i >= 0; -- i )                        // Iterate newest (topmost) first
        {
            if( no_position )
            {
                if( elements[ i ] -> acceptEvent( e ) )
                    break;
            }
            else
            {
                element_position   = elements[ i ] -> getVisualPosition();
                element_dimensions = elements[ i ] -> getVisualDimensions();
                
                if( ( e.type == STROKE
                      && pointInsideRect( e.stroke.prev_pos[ 0 ],
                                          e.stroke.prev_pos[ 1 ],
                                          element_position.first,
                                          element_position.second,
                                          element_dimensions.first,
                                          element_dimensions.second ) )
                    || pointInsideRect( e_position[ 0 ],
                                        e_position[ 1 ],
                                        element_position.first,
                                        element_position.second,
                                        element_dimensions.first,
                                        element_dimensions.second ) )
                {
                    if( elements[ i ] -> acceptEvent( e ) )
                        break;
                }
            }
        }
    }
    
    bqt_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    void window::associateDevice( bqt_platform_idevid_t dev_id,
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
    void window::deassociateDevice( bqt_platform_idevid_t dev_id )
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        if( !input_assoc.erase( dev_id ) && getDevMode() )
            ff::write( bqt_out, "Warning: Attempt to deassociate a non-associated device\n" );
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
        
        target -> window_mutex.lock();                                     // We need to explicitly lock/unlock this as the window can be destroyed
        
        if( !( target -> platform_window.good ) )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( target -> updates.close )
        {
            /* GUI CLEANUP ****************************************************//******************************************************************************/
            
            for( int i = 0; i < target -> elements.size(); ++i )                          // Deletes all gui elements
                delete target -> elements[ i ];
            
            target -> elements.clear();
            
            /* WINDOW CLEANUP *************************************************//******************************************************************************/
            
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
                    {
                        redraw_window = true;
                    }
                    
                    target -> updates.dimensions = false;
                }
                
                if( target -> updates.active )
                {
                    // target -> updates.restore = true;
                    
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
                    ff::write( bqt_out, "window::manipulate::execute(): Centering not implemented yet, ignoring\n" );
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
                    ff::write( bqt_out, "window::manipulate::execute(): Maximize not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Maximize not implemented
                    
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
    void window::manipulate::makeActive()
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        target -> updates.active = true;
        
        target -> updates.changed = true;
    }
    
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
            
            // glLoadIdentity();
            glViewport( 0, 0, target.dimensions[ 0 ], target.dimensions[ 1 ] );
            glLoadIdentity();
            glOrtho( 0.0, target.dimensions[ 0 ], target.dimensions[ 1 ], 0.0, 1.0, -1.0 );
            
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
            // glEnable( GL_DEPTH_TEST );
            glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
            glClearStencil( 0 );
            // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
            
            glEnable( GL_TEXTURE_2D );
            
            // glEnable( GL_POLYGON_SMOOTH );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            
            for( int i = 0; i < target.elements.size(); ++i )
                target.elements[ i ] -> draw();
            
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


