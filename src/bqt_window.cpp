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
#include "bqt_png.hpp"
#include "gui/bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // WINDOW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
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
        
        initOpenGL();
        
        initNamedResources();
    }
    
    void window::makeContextCurrent()
    {
        Display* x_display = getXDisplay();
        glXMakeCurrent( x_display,
                        platform_window.x_window,
                        platform_window.glx_context );
        
        XFlush( x_display );
    }
    
    void window::associateDevice( bqt_platform_idevid_t dev_id,
                                  layout_element* element )
    {
        scoped_lock< rwlock > scoped_lock( window_lock, RW_WRITE );
        
        if( element == NULL && input_assoc.count( dev_id ) )
            input_assoc.erase( dev_id );
        else
            input_assoc[ dev_id ] = element;
    }
    
    void window::initNamedResources()
    {
        
    }
    
    void window::openUnopenedTextureFiles()
    {
        if( new_textures )
        {
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 ++iter )
            {
                if( iter -> second -> data == NULL
                    && iter -> second -> texture -> gl_texture == 0x00 )
                {
                    png_file rsrc_file( iter -> first );
                    
                    std::pair< unsigned int, unsigned int > rsrc_dim = rsrc_file.getDimensions();
                    
                    iter -> second -> data = new unsigned char[ rsrc_dim.first * rsrc_dim.second * 4 ];
                    
                    if( iter -> second -> data == NULL )
                        throw bqt::exception( "window::openUnopenedTextureFiles(): Could not allocate conversion space" );
                    
                    rsrc_file.toRGBABytes( iter -> second -> data );
                    
                    iter -> second -> texture -> dimensions[ 0 ] = rsrc_dim.first;
                    iter -> second -> texture -> dimensions[ 1 ] = rsrc_dim.second;
                    
                    // ff::write( bqt_out,
                    //            "Opening \"",
                    //            iter -> first,
                    //            "\" as a resource texture\n" );
                }
            }
        }
    }
    void window::uploadUnuploadedTextures()
    {
        if( new_textures )
        {
            makeContextCurrent();
            
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 ++iter )
            {
                if( iter -> second -> texture -> gl_texture == 0x00
                    && iter -> second -> data != NULL )
                {
                    glGenTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                    
                    if( iter -> second -> texture -> gl_texture == 0x00 )
                        throw exception( "window::uploadUnuploadedTextures(): Could not generate texture" );
                    
                    glBindTexture( GL_TEXTURE_2D, iter -> second -> texture -> gl_texture );
                    glTexImage2D( GL_TEXTURE_2D,
                                  0,
                                  GL_RGBA,
                                  iter -> second -> texture -> dimensions[ 1 ],
                                  iter -> second -> texture -> dimensions[ 0 ],
                                  0,
                                  GL_RGBA,
                                  GL_UNSIGNED_BYTE,
                                  iter -> second -> data );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                    
                    glBindTexture( GL_TEXTURE_2D, 0 );
                    
                    GLenum gl_error = glGetError();
                    if( gl_error != GL_NO_ERROR )
                    {
                        bqt::exception e;
                        ff::write( *e,
                                   "LoadGUIResource_task::execute(): OpenGL error 0x",
                                   ff::to_x( ( unsigned long )gl_error ),
                                   " loading pixels from 0x",
                                   ff::to_x( ( unsigned long )( iter -> second -> data ), HEX_WIDTH, HEX_WIDTH ),
                                   " to texture 0x",
                                   ff::to_x( iter -> second -> texture -> gl_texture, HEX_WIDTH, HEX_WIDTH ) );
                        throw e;
                    }
                    
                    // ff::write( bqt_out,
                    //            "Creating \"",
                    //            iter -> first,
                    //            "\" as a resource texture 0x",
                    //            ff::to_x( iter -> second -> texture -> gl_texture ),
                    //            "\n" );
                    
                    delete[] iter -> second -> data;
                    iter -> second -> data = NULL;
                }
            }
            
            new_textures = false;
        }
    }
    void window::deleteUnreferencedTextures()
    {
        if( old_textures )
        {
            makeContextCurrent();
            
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 /* NULL */ )
            {
                if( iter -> second -> ref_count < 1 )
                {
                    // ff::write( bqt_out,
                    //            "Deleting \"",
                    //            iter -> first,
                    //            "\" as a resource texture 0x",
                    //            ff::to_x( iter -> second -> texture -> gl_texture ),
                    //            "\n" );
                    
                    if( iter -> second -> texture -> gl_texture != 0x00 )
                        glDeleteTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                    
                    delete iter -> second;
                    
                    resource_textures.erase( iter++ );
                }
                else
                    ++iter;
            }
            
            old_textures = false;
        }
    }
    
    gui_texture* window::acquireTexture( std::string filename )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        gui_texture_holder* h;
        
        if( resource_textures.count( filename ) )
            h = resource_textures[ filename ];
        else
        {
            h = new gui_texture_holder();
            resource_textures[ filename ] = h;
            new_textures = true;
        }
        
        h -> ref_count++;
        
        // ff::write( bqt_out,
        //            "Acquiring texture \"",
        //            filename,
        //            "\" as 0x",
        //            ff::to_x( ( unsigned long )( h -> texture ) ),
        //            " (rc ",
        //            h -> ref_count,
        //            ")\n" );
        
        return h -> texture;
    }
    void window::releaseTexture( gui_texture* t )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
             iter != resource_textures.end();
             ++iter )
        {
            if( iter -> second -> texture == t )
            {
                iter -> second -> ref_count--;
                
                // ff::write( bqt_out,
                //            "Releasing texture \"",
                //            iter -> first,
                //            "\" as 0x",
                //            ff::to_x( ( unsigned long )( iter -> second -> texture ) ),
                //            " (rc ",
                //            iter -> second -> ref_count,
                //            ")\n" );
                
                if( iter -> second -> ref_count < 1 )
                {
                    old_textures = true;
                    
                    if( iter -> second -> ref_count < 0 )
                        throw exception( "window::releaseTexture(): Texture reference count < 0" );
                }
                
                return;
            }
        }
        
        throw exception( "window::releaseTexture(): No such texture" );
    }
    
    // gui_resource* window::getNamedResource( gui_resource_name name )
    // {
        
    // }
    
    window::~window()
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );                   // If we really need this we have bigger problems
        
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
    
    window::window() // : gui( this, BQT_WINDOW_DEFAULT_WIDTH, BQT_WINDOW_DEFAULT_HEIGHT )
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
        updates.redraw     = false;
        
        new_textures = false;
        old_textures = false;
    }
    
    std::pair< unsigned int, unsigned int > window::getDimensions()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< int, int > window::getPosition()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        switch( e.type )
        {
        case STROKE:
            e.stroke.position[ 0 ] -= position[ 0 ];
            e.stroke.position[ 1 ] -= position[ 1 ];
            break;
        case DROP:
            e.drop.position[ 0 ] -= position[ 0 ];
            e.drop.position[ 1 ] -= position[ 1 ];
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            // No adjustment needed for these
            break;
        case PINCH:
            e.pinch.position[ 0 ] -= position[ 0 ];
            e.pinch.position[ 1 ] -= position[ 1 ];
            break;
        default:
            throw exception( "window::acceptEvent(): Unknown event type" );
            break;
        }
        
        // Devel
        if( e.type == KEYCOMMAND && e.key.key == KEY_Q && e.key.cmd && e.key.up )
            setQuitFlag();
        
        if( e.type == KEYCOMMAND && e.key.key == KEY_T && e.key.up )
            acquireTexture( "make/BQTDraw/Resources/gui_resources.png" );
        
        if( e.type == KEYCOMMAND && e.key.key == KEY_Y && e.key.up )
            releaseTexture( resource_textures.begin() -> second -> texture );
        
        manipulate* m = new manipulate( this );
        m -> redraw();
        submitTask( m );
    }
    
    bqt_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        
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
        
        target -> window_lock.lock_write();                                          // We need to explicitly lock/unlock this as the window can be destroyed
        
        if( !( target -> platform_window.good ) )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( target -> updates.close )
        {
            #warning window::manipulate::execute() does not clean up GUI
            
            // if( !( target -> gui.isClean() ) )
            // {
            //     target -> gui.startClean();
            //     return false;                                                   // Requeue if there are still resources to clean up
            // }
            
            deregisterWindow( *target );
            target -> window_lock.unlock();
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
            target -> openUnopenedTextureFiles();
            
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
                        // target -> gui.setDimensions( target -> dimensions[ 0 ],
                        //                              target -> dimensions[ 1 ] );   // Update the gui dimensions
                        
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
            
            target -> window_lock.unlock();
            
            if( redraw_window )
                submitTask( new window::redraw( *target ) );
        }
        
        return true;
    }
    
    void window::manipulate::setDimensions( unsigned int w, unsigned int h )
    {
        if( w < 1 || h < 1 )
            throw exception( "window::manipulate::setDimensions(): Width or height < 1" );
        
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> dimensions[ 0 ] = w;
        target -> dimensions[ 1 ] = h;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( int x, int y )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> position[ 0 ] = x;
        target -> position[ 1 ] = y;
        target -> updates.position = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::setFullscreen( bool f )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> fullscreen = true;
        target -> updates.fullscreen = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setTitle( std::string t )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> title = t;
        target -> updates.title = true;
        
        target -> updates.changed = true;
    }
    
    // void window::manipulate::setFocus( bool f )
    // {
    //     scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
    //     target -> in_focus = true;
    // }
    void window::manipulate::makeActive()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.active = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::center()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.center = true;                                        // Don't calculate here, as that code may be thread-dependent
        
        target -> updates.changed = true;
    }
    void window::manipulate::minimize()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.minimize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::maximize()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.maximize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::restore()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.restore = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::close()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.close = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::redraw()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.redraw = true;
        
        target -> updates.changed = true;
    }
    
    // WINDOW::REDRAW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock< rwlock > slock( target.window_lock, RW_WRITE );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        scoped_lock< rwlock > slock( target.window_lock, RW_READ );
        
        target.makeContextCurrent();
        
        target.deleteUnreferencedTextures();
        target.uploadUnuploadedTextures();
        
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
            
            glClearColor( 0.3, 0.3, 0.3, 1.0 );
            glClear( GL_COLOR_BUFFER_BIT );
            
            glEnable( GL_TEXTURE_2D );
            
            {
                for( std::map< std::string, gui_texture_holder* >::iterator iter = target.resource_textures.begin();
                     iter != target.resource_textures.end();
                     ++iter )
                {
                    glBindTexture( GL_TEXTURE_2D, iter -> second -> texture -> gl_texture );
                    
                    glBegin( GL_QUADS );
                    {
                        glTexCoord2f( 0.0, 0.0 );
                        glVertex2i( 0, 0 );
                        
                        glTexCoord2f( 0.0, 1.0 );
                        glVertex2i( 0, iter -> second -> texture -> dimensions[ 1 ] );
                        
                        glTexCoord2f( 1.0, 1.0 );
                        glVertex2i( iter -> second -> texture -> dimensions[ 0 ], iter -> second -> texture -> dimensions[ 1 ] );
                        
                        glTexCoord2f( 1.0, 0.0 );
                        glVertex2i( iter -> second -> texture -> dimensions[ 0 ], 0 );
                    }
                    glEnd();
                }
            }
            
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


