/* 
 * jb_window.cpp
 * 
 * Implements jb_window.hpp
 * 
 */

// http://www.opengl.org/wiki/Texture_Storage#Texture_copy

// http://tronche.com/gui/x/xlib/pixmap-and-cursor/cursor.html
// http://tronche.com/gui/x/xlib/window/XDefineCursor.html

// TODO: Invesitigate how to group windows in e.g. the Ubuntu task bar, possibly XWMHINTS WindowGroupHint
// http://tronche.com/gui/x/xlib/ICC/client-to-window-manager/wm-hints.html

// Also a note about setting window icons on 32 vs. 54 bit
// http://stackoverflow.com/questions/10699927/xlib-argb-window-icon

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

#include <cmath>

#include "jb_windowevent.hpp"
#include "jb_windowmanagement.hpp"
#include "../gui/jb_named_resources.hpp"
#include "../gui/jb_windowview.hpp"
#include "../tasking/jb_taskexec.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    window::window() : top_element( new windowview( this ) )
    {
        // We create the top_element on construction rather than init() as we
        // might get a call for getTopGroup() before init().
        
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
        dpi::percent scale = getScaleFactor();
        
        dimensions[ 0 ] = def_dim.first  * scale;
        dimensions[ 1 ] = def_dim.second * scale;
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
        updates.center     = false;
        updates.minimize   = false;
        updates.maximize   = false;
        updates.restore    = false;
        updates.redraw     = false;
        
        can_add_containers = true;
    }
    
    std::pair< dpi::points, dpi::points > window::getDimensions()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        dpi::percent scale = getScaleFactor();
        
        return std::pair< dpi::points,
                          dpi::points >( dimensions[ 0 ] / scale,
                                         dimensions[ 1 ] / scale );
    }
    std::pair< dpi::points, dpi::points > window::getPosition()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        dpi::percent scale = getScaleFactor();
        
        return std::pair< dpi::points,
                          dpi::points >( position[ 0 ] / scale,
                                         position[ 1 ] / scale );
    }
    std::pair< dpi::pixels, dpi::pixels > window::getPxDimensions()
    {
        scoped_lock< mutex > slock( window_mutex );
        return std::pair< dpi::points,
                          dpi::points >( dimensions[ 0 ],
                                         dimensions[ 1 ] );
    }
    std::pair< dpi::pixels, dpi::pixels > window::getPxPosition()
    {
        scoped_lock< mutex > slock( window_mutex );
        return std::pair< dpi::points,
                          dpi::points >( position[ 0 ],
                                         position[ 1 ] );
    }
    dpi::pixels window::getDPI()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        #if defined PLATFORM_XWS_GNUPOSIX
        
        {
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
        
        #else
        
        #warning DPI detection not supported on this platform
        
        return STANDARD_DPI;
        
        #endif
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
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( window_mutex );
        
////////// DEVEL: //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if( e.type == KEYCOMMAND && e.key.key == KEY_Q && e.key.cmd && e.key.up )
        {
            jb_setQuitFlag();
            return;
        }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // TODO: Move to jade::windowview
        
        dpi::percent scale = getScaleFactor();
        
        bool no_position = false;
        
        // int e_position[ 2 ];
        
        e.offset[ 0 ] = 0;
        e.offset[ 1 ] = 0;
        
        switch( e.type )                                                        // Normalize event position to window position (if necessary)
        {
        case STROKE:
            e.stroke.position[ 0 ] -= position[ 0 ] / scale;
            e.stroke.position[ 1 ] -= position[ 1 ] / scale;
            e.stroke.prev_pos[ 0 ] -= position[ 0 ] / scale;
            e.stroke.prev_pos[ 1 ] -= position[ 1 ] / scale;
            break;
        case DROP:
            e.drop.position[ 0 ] -= position[ 0 ] / scale;
            e.drop.position[ 1 ] -= position[ 1 ] / scale;
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            no_position = true;
            break;
        case PINCH:
            e.pinch.position[ 0 ] -= position[ 0 ] / scale;
            e.pinch.position[ 1 ] -= position[ 1 ] / scale;
            break;
        case SCROLL:
            e.scroll.position[ 0 ] -= position[ 0 ] / scale;
            e.scroll.position[ 1 ] -= position[ 1 ] / scale;
            break;
        default:
            throw exception( "window::acceptEvent(): Unknown event type" );
            break;
        }
        
        if( e.type == STROKE )                                                  // Check elements in device association list first
        {
            auto finder = input_assoc.find( e.stroke.dev_id );
            if( finder != input_assoc.end() )
            {
                auto iter = finder -> second.chain.begin();
                
                auto last = --( finder -> second.chain.end() );
                
                if( iter == last )
                    throw exception( "window::acceptEvent(): Zero-length association chain" );
                
                while( iter != last )
                {
                    auto pos = ( *iter ) -> getRealPosition();
                    e.offset[ 0 ] = pos.first;
                    e.offset[ 1 ] = pos.second;
                    ++iter;
                }
                
                ( *last ) -> acceptEvent( e );
                
                return;
            }
        }
        
        top_element -> acceptEvent( e );                                        // Send event to top-level element (at 0,0; dimensions match window)
    }
    
    jb_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    std::string window::getTitle()
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        return title;
    }
    
    std::shared_ptr< windowview > window::getTopElement()
    {
        return top_element;                                                     // No thread safety required, as it has the same lifetime as the window
    }
    
    void window::requestRedraw()
    {
        scoped_lock< mutex > slock( redraw_mutex );
        
        if( pending_redraws < 1 )
        {
            submitTask( new redraw( *this ) );
        }
    }
    
    void window::register_container( container< window >* c )
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !can_add_containers )
        {
            #ifdef JADEBASE_WINDOW_CONTAINERREGISTERSILENTFAIL
            c -> clear();
            #else
            throw exception( "window::register_container(): Cannot register containers now" );
            #endif
        }
        else
            containers.insert( c );
    }
    void window::deregister_container( container< window >* c )
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( !containers.erase( c ) )                                            // Might as well throw an exception as something's wrong in the calling code
            throw exception( "window::deregister_container(): Container not a child" );
    }
    
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
            
            glXMakeCurrent( x_display,
                            platform_window.x_window
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
            XFree( platform_window.x_visual_info );
            
            platform_window.good = false;
        }
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
    }
    
    void window::associateDevice( jb_platform_idevid_t dev_id,
                                  std::list< gui_element* >& chain )
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        if( chain.empty() )
        {
            if( getDevMode() )
                ff::write( jb_out,
                           "Notice: window::associateDevice(): Empty assoc chain" );
            return;
        }
        
        idev_assoc& assoc( input_assoc[ dev_id ] );
        
        std::swap( assoc.chain, chain );
    }
    void window::deassociateDevice( jb_platform_idevid_t dev_id )
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        if( !input_assoc.erase( dev_id ) && getDevMode() )
            ff::write( jb_out, "Warning: Attempt to deassociate a non-associated device\n" );
    }
    
    /* window::manipulate *****************************************************//******************************************************************************/
    
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
    
    void window::manipulate::setDimensions( dpi::points w, dpi::points h )
    {
        if( w < 1 || h < 1 )
            throw exception( "window::manipulate::setDimensions(): Width or height < 1" );
        
        scoped_lock< mutex > slock( target -> window_mutex );
        
        dpi::percent scale = target -> getScaleFactor();
        
        target -> dimensions[ 0 ] = w * scale;
        target -> dimensions[ 1 ] = h * scale;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( target -> window_mutex );
        
        dpi::percent scale = target -> getScaleFactor();
        
        target -> position[ 0 ] = x * scale;
        target -> position[ 1 ] = y * scale;
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
    
    /* window::redraw *********************************************************//******************************************************************************/
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock< mutex > slock( target.redraw_mutex );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        target.redraw_mutex.lock();
        
        target.makeContextCurrent();
        
        if( target.pending_redraws <= 1 )                                       // Only redraw if there are no other pending redraws for that window; this is
                                                                                // safe because the redraw task is high-priority, so the task system will
                                                                                // eventually drill down to the last one.
        {
            if( target.pending_redraws != 1 )                                   // Sanity check
                throw exception( "window::redraw::execute(): Target pending redraws somehow < 1" );
            
            target.pending_redraws--;
            target.redraw_mutex.unlock();
            
            scoped_lock< mutex > slock( target.window_mutex );
            
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
            
            glColor4f( 1.0, 1.0f, 1.0f, 1.0f );
            
            dpi::percent scale = target.getScaleFactor();
            glScalef( scale, scale, 1.0f );
            
            target.top_element -> draw( &target );
            
            #if defined PLATFORM_XWS_GNUPOSIX
            
            Display* x_display = getXDisplay();
            glXSwapBuffers( x_display, target.platform_window.x_window );
            
            #else
            
            #error "Buffer swapping not implemented on non-X platforms"
            
            #endif
        }
        else
        {
            target.pending_redraws--;
            target.redraw_mutex.unlock();
        }
        
        return true;
    }
}


