/* 
 * jb_window.cpp
 * 
 * Implements platform-agnostic parts of jb_window.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

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

// Utility macros for manipulator functions until there's a better solution
#define GET_MANIPULATE_m    scoped_lock< mutex > slock( window_mutex );\
                            \
                            window::ManipulateWindow_task m;\
                            \
                            if( pending_manip == nullptr )\
                                m = new window::ManipulateWindow_task( this );\
                            else\
                                m = pending_manip;
#define SUBMIT_MANIPULATE_m if( pending_manip == nullptr )\
                            {\
                                submitTask( m );\
                                pending_manip = m;\
                            }

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    window::window() : input_assoc( jb_platform_idevid_t_less ),
                       top_element( new windowview( this ) )
    {
        // We create the top_element on construction rather than init() as we
        // might get a call for getTopGroup() before init().
        
        window::platformWindowConstructor();
        
        pending_redraw = false;
        
        pending_manip = nullptr;
        
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
        
        submitTask( new ManipulateWindow_task( this ) );                        // Initializes the platform window
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
    void window::setDimensions( dpi::points w, dpi::points h )
    {
        GET_MANIPULATE_m;
        
        m -> update.dimensions = true;
        m -> dimensions[ 0 ] = w;
        m -> dimensions[ 1 ] = h;
        
        SUBMIT_MANIPULATE_m;
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
    void window::setPosition( dpi::points x, dpi::points y )
    {
        GET_MANIPULATE_m;
        
        m -> update.position = true;
        m -> position[ 0 ] = x;
        m -> position[ 1 ] = y;
        
        SUBMIT_MANIPULATE_m;
    }
    
    std::string window::getTitle()
    {
        scoped_lock< mutex > scoped_lock( window_mutex );
        
        return title;
    }
    void window::setTitle( std::string t )
    {
        GET_MANIPULATE_m;
        
        m -> update.title = true;
        m -> title = t;
        
        SUBMIT_MANIPULATE_m;
    }
    
    void window::setFullscreen( bool f )
    {
        GET_MANIPULATE_m;
        
        m -> update.fullscreen = true;
        m -> fullscreen = f;
        
        SUBMIT_MANIPULATE_m;
    }
    void window::center()
    {
        GET_MANIPULATE_m;
        
        m -> update.center = true;
        
        SUBMIT_MANIPULATE_m;
    }
    void window::minimize()
    {
        GET_MANIPULATE_m;
        
        m -> update.minimize = true;
        
        SUBMIT_MANIPULATE_m;
    }
    void window::maximize()
    {
        GET_MANIPULATE_m;
        
        m -> update.maximize = true;
        
        SUBMIT_MANIPULATE_m;
    }
    void window::restore()
    {
        GET_MANIPULATE_m;
        
        m -> update.restore = true;
        
        SUBMIT_MANIPULATE_m;
    }
    void window::close()
    {
        GET_MANIPULATE_m;
        
        m -> update.close = true;
        
        SUBMIT_MANIPULATE_m;
    }
    
    void window::requestRedraw()
    {
        scoped_lock< mutex > slock( redraw_mutex );
        
        if( !pending_redraw )
        {
            submitTask( new redraw( *this ) );
            pending_redraw = true;
        }
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
        
        // Window position is stored in pixels, so we need this to scale the
        // position for normalization.
        dpi::percent scale = getScaleFactor();
        
        switch( e.type )                                                        // Normalize event position to window position (if necessary)
        {
        case STROKE:
            e.stroke.prev_pos[ 0 ] -= position[ 0 ] / scale;
            e.stroke.prev_pos[ 1 ] -= position[ 1 ] / scale;
        case DROP:
        case PINCH:
        case SCROLL:
            e.position[ 0 ] -= position[ 0 ] / scale;
            e.position[ 1 ] -= position[ 1 ] / scale;
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
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
                
                if( iter == finder -> second.chain.end() )
                    throw exception( "window::acceptEvent(): Zero-length association chain" );
                
                while( iter != finder -> second.chain.end() )
                {
                    auto pos = ( *iter ) -> getEventOffset();
                    
                    switch( e.type )                                       // Normalize event position to element position (if necessary)
                    {
                    case STROKE:
                        e.stroke.prev_pos[ 0 ] -= pos.first;
                        e.stroke.prev_pos[ 1 ] -= pos.second;
                    case DROP:
                    case PINCH:
                    case SCROLL:
                        e.position[ 0 ] -= pos.first;
                        e.position[ 1 ] -= pos.second;
                        break;
                    default:
                        // All other cases already handled
                        break;
                    }
                    
                    ++iter;
                }
                
                auto last = --( finder -> second.chain.end() );
                ( *last ) -> acceptEvent( e );
                
                return;
            }
        }
        
        top_element -> acceptEvent( e );                                        // Send event to top-level element (at 0,0; dimensions match window)
    }
    
    std::shared_ptr< windowview > window::getTopElement()
    {
        return top_element;                                                     // No thread safety required, as it has the same lifetime as the window
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
    
    /* window::ManipulateWindow_task ******************************************//******************************************************************************/
    
    window::ManipulateWindow_task::ManipulateWindow_task( window* t )
    {
        if( t == NULL )
            target = new window();                                              // Useful for debugging, or if we just need a window no matter what dammit
        else
            target = t;
    }
    
    bool window::ManipulateWindow_task::execute( task_mask* caller_mask )
    {
        bool redraw_window = false;
        
        target -> window_mutex.lock();                                          // We need to explicitly lock/unlock this as the window can be destroyed
        
        if( windowNeedsInit() )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( updates.close )
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
        }
        else
        {
            if( updates.dimensions )
            {
                // Fluid syncing of window & top_element dimensions:
                //   1. Window dimension change requested
                //   2. Window then sets top_element dimensions
                //   3. top_element clips dimensions to calculated mins (if any)
                //   4. Window then gets top_element dimensions and uses those
                //      for actual dimensions to update to
                //   => No min window dimensions (X will be happier)
                
                if( dimensions[ 0 ] < 0 )
                    dimensions[ 0 ] = 0;
                if( dimensions[ 1 ] < 0 )
                    dimensions[ 1 ] = 0;
                
                top = target -> top_element;
                
                top -> setRealDimensions( dimensions[ 0 ] / scale,
                                          dimensions[ 1 ] / scale );
                
                std::pair< dpi::points, dpi::points > limits = top -> getRealDimensions();
                
                if( dimensions[ 0 ] < limits.first )
                    dimensions[ 0 ] = limits.first;
                if( dimensions[ 1 ] < limits.second )
                    dimensions[ 1 ] = limits.second;
                
                updateDimensions();
                
                dpi::percent scale = target -> getScaleFactor();
                
                target -> dimensions[ 0 ] = dimensions[ 0 ] * scale;
                target -> dimensions[ 1 ] = dimensions[ 1 ] * scale;
                
                redraw_window = true;
            }
            
            if( updates.position )
            {
                updatePosition();
                
                dpi::percent scale = target -> getScaleFactor();
                
                target -> position[ 0 ] = position[ 0 ] * scale;
                target -> position[ 1 ] = position[ 1 ] * scale;
                
                redraw_window = true;
            }
            
            if( updates.fullscreen
                && target -> fullscreen != fullscreen )
            {
                updateFullscreen();
                
                target -> fullscreen = fullscreen;
                
                redraw_window = true;
            }
            
            if( updates.title )
            {
                updateTitle();
                
                target -> title = title;
                
                redraw_window = true;
            }
            
            if( updates.center
                && !( target -> fullscreen ) )
            {
                updateCenter();
                redraw_window = true;
            }
            
            // TODO: A minimized/restored flag so we only call these when
            // they're changed
            
            if( updates.minimize
                && !( target -> fullscreen ) )
            {
                updateMinimize();
                redraw_window = true;
            }
            
            if( updates.maximize
                && !( target -> fullscreen ) )
            {
                updateMaximize();
                redraw_window = true;
            }
            
            if( updates.restore
                /* && !( target -> fullscreen ) */ )
            {
                updateRestore();
                redraw_window = true;
            }
            
            target -> window_mutex.unlock();
            
            if( redraw_window )
                submitTask( new window::redraw( *target ) );
        }
        
        return true;
    }
    
    /* window::redraw *********************************************************//******************************************************************************/
    
    window::redraw::redraw( window& t ) : target( t )
    {
        // scoped_lock< mutex > slock( target.redraw_mutex );
        
        // target.pending_redraw = true;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        {
            scoped_lock< mutex > rlock( target.redraw_mutex );
            
            target.pending_redraw = false;
        }
        
        scoped_lock< mutex > slock( target.window_mutex );
        
        target.makeContextCurrent();
        
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
        
        target.swapBuffers();
        
        return true;
    }
}


