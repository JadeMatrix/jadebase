/* 
 * jb_window.cpp
 * 
 * Implements platform-agnostic parts of jb_window.hpp
 * 
 */

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
        
        pending_redraw = false;
        
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
        
        if( !pending_redraw )
        {
            submitTask( new redraw( *this ) );
            pending_redraw = true;
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


