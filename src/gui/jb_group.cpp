/* 
 * jb_group.cpp
 * 
 * Implements jb_group.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_group.hpp"

#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"
#include "../windowsys/jb_window.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    group::group( dpi::points x,
                  dpi::points y,
                  dpi::points w,
                  dpi::points h ) : scrollable( x, y, w, h )
    {
        draw_background = true;
        
        stroke_fallthrough = false;
        
        scroll_offset[ 0 ] = 0;
        scroll_offset[ 1 ] = 0;
        
        updateScrollParams();
    }
    
    void group::addElement( const std::shared_ptr< gui_element >& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( !e )
            throw exception( "group::addElement(): Empty shared_ptr" );
        
        e -> setParentElement( this );
        elements.push_back( e );
        
        updateScrollParams();
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void group::removeElement( const std::shared_ptr< gui_element >& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( !e )
            throw exception( "group::removeElement(): Empty shared_ptr" );
        
        for( auto iter = elements.begin();
             iter != elements.end();
             ++iter )
        {
            if( *iter == e )
            {
                elements.erase( iter );                                         // Dereferences by deleting std::shared_ptr
                
                e -> setParentElement( NULL );
                
                updateScrollParams();
                
                if( parent != NULL )
                    parent -> requestRedraw();
                
                return;
            }
        }
        
        throw exception( "group::removeElement(): No such element" );
    }
    
    void group::setDrawBackground( bool d )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        draw_background = d;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    bool group::getDrawBackground()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return draw_background;
    }
    
    void group::setStrokeFallthrough( bool t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        stroke_fallthrough = t;
    }
    bool group::getStrokeFallthrough()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return stroke_fallthrough;
    }
    
    // CALLBACKS & EVENTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void group::setShownCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        shown_callback = cb;
    }
    void group::setHiddenCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        hidden_callback = cb;
    }
    void group::setClosedCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        closed_callback = cb;
    }
    
    void group::shown()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( shown_callback )
            shown_callback -> call();
    }
    void group::hidden()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( hidden_callback )
            hidden_callback -> call();
    }
    void group::closed()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( closed_callback )
            closed_callback -> call();
    }
    
    // GUI_ELEMENT /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void group::setRealPosition( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void group::setRealDimensions( dpi::points w, dpi::points h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        updateScrollParams();
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    std::pair< dpi::points, dpi::points > group::getVisualPosition()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::pair< dpi::points, dpi::points > v_position = getRealPosition();
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< dpi::points, dpi::points > evp = elements[ i ] -> getVisualPosition();
            
            if( evp.first < v_position.first )
                v_position.first = evp.first;
            
            if( evp.second < v_position.second )
                v_position.second = evp.second;
        }
        
        return v_position;
    }
    std::pair< dpi::points, dpi::points > group::getEventOffset()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::points,
                          dpi::points >( position[ 0 ] - scroll_offset[ 0 ],
                                         position[ 1 ] - scroll_offset[ 1 ] );
    }
    std::pair< dpi::points, dpi::points > group::getVisualDimensions()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::pair< dpi::points, dpi::points > v_position = getVisualPosition();
        std::pair< dpi::points, dpi::points > v_dimensions = getRealDimensions();
        
        if( v_position.first < position[ 0 ] )                                  // Always guaranteed to be <= from getVisualPosition()
            v_dimensions.first += position[ 0 ] - v_position.first;
        if( v_position.second < position[ 1 ] )
            v_dimensions.second += position[ 1 ] - v_position.second;
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< dpi::points, dpi::points > evp = elements[ i ] -> getVisualPosition();
            std::pair< dpi::points, dpi::points > evd = elements[ i ] -> getVisualDimensions();
            
            if( evp.first + evd.first > v_position.first + v_dimensions.first )
                v_dimensions.first = evd.first;
            
            if( evp.second + evd.second > v_position.second + v_dimensions.second )
                v_dimensions.second = evd.second;
        }
        
        return v_dimensions;
    }
    
    bool group::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        for( int i = elements.size() - 1; i >= 0; -- i )                        // Iterate newest (topmost) first
        {
            if( sendEventToChild( i, e ) )
                return true;
        }
        
        if( e.type == STROKE )
        {
            if( stroke_fallthrough )
                return false;
            else
                return !pointInsideRect( e.position[ 0 ],
                                         e.position[ 1 ],
                                         0,
                                         0,
                                         dimensions[ 0 ],
                                         dimensions[ 1 ] );
        }
        else
            return false;
    }
    
    void group::draw( window* w )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            if( draw_background )
            {
                glBegin( GL_QUADS );
                {
                    glColor4f( 0.3f, 0.3f, 0.3f, 1.0f );
                    
                    glVertex2f( 0.0f, 0.0f );
                    glVertex2f( 0.0f, dimensions[ 1 ] );
                    glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                    glVertex2f( dimensions[ 0 ], 0.0f );
                    
                    glColor4f( 1.0, 1.0f, 1.0f, 1.0f );
                }
                glEnd();
            }
            
            clearDrawMasks();
            
            for( int i = 0; i < elements.size(); ++i )
            {
                addDrawMask( 0, 0, dimensions[ 0 ], dimensions[ 1 ] );          // Do it for every element, as they might erase the mask
                
                glTranslatef( -scroll_offset[ 0 ], -scroll_offset[ 1 ], 0 );
                {
                    elements[ i ] -> draw( w );
                }
                glTranslatef( scroll_offset[ 0 ], scroll_offset[ 1 ], 0 );
            }
            
            clearDrawMasks();
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
    
    void group::clearDeviceAssociations()                                       // clearDeviceAssociations() is not required to be thread-safe
    {
        for( int i = 0; i < elements.size(); ++i )
            elements[ i ] -> clearDeviceAssociations();
    }
    
    // SCROLLABLE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void group::scrollPoints( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scroll_offset[ 0 ] += x;
        scroll_offset[ 1 ] += y;
        
        clampScroll();
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void group::scrollPercent( dpi::percent x, dpi::percent y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPoints( x * dimensions[ 0 ], y * dimensions[ 1 ] );
    }
    
    void group::setScrollPoints( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPoints( x - scroll_offset[ 0 ], y - scroll_offset[ 1 ] );
    }
    void group::setScrollPercent( dpi::percent x, dpi::percent y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPoints( x * dimensions[ 0 ] - scroll_offset[ 0 ],
                      y * dimensions[ 1 ] - scroll_offset[ 1 ] );
    }
    
    std::pair< dpi::points, dpi::points > group::getScrollPoints()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::points, dpi::points >( scroll_offset[ 0 ], scroll_offset[ 1 ] );
    }
    std::pair< dpi::percent, dpi::percent > group::getScrollPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::percent,
                          dpi::percent >( scroll_offset[ 0 ] / dimensions[ 0 ],
                                          scroll_offset[ 1 ] / dimensions[ 1 ] );
    }
    
    bool group::hasScrollLimit()
    {
        return true;
    }
    std::pair< dpi::points, dpi::points > group::getScrollLimitPoints()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::points, dpi::points >( scroll_limits[ 0 ], scroll_limits[ 1 ] );
    }
    std::pair< dpi::percent, dpi::percent> group::getScrollLimitPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::percent,
                          dpi::percent >( scroll_limits[ 0 ] / dimensions[ 0 ],
                                          scroll_limits[ 1 ] / dimensions[ 1 ] );
    }
    
    bool group::sendEventToChild( int i, window_event e )
    {
        auto element_position   = elements[ i ] -> getVisualPosition();
        auto element_dimensions = elements[ i ] -> getVisualDimensions();
        
        if( ( e.type == STROKE                                                  // Allow elements to see exiting strokes
              && pointInsideRect( e.stroke.prev_pos[ 0 ],
                                  e.stroke.prev_pos[ 1 ],
                                  element_position.first  - scroll_offset[ 0 ],
                                  element_position.second - scroll_offset[ 1 ],
                                  element_dimensions.first,
                                  element_dimensions.second ) )
            || pointInsideRect( e.position[ 0 ],
                                e.position[ 1 ],
                                element_position.first  - scroll_offset[ 0 ],
                                element_position.second - scroll_offset[ 1 ],
                                element_dimensions.first,
                                element_dimensions.second ) )
        {
            auto element_offset = elements[ i ] -> getEventOffset();
            
            switch( e.type )
            {
            case STROKE:
                e.stroke.prev_pos[ 0 ] -= element_offset.first  - scroll_offset[ 0 ];
                e.stroke.prev_pos[ 1 ] -= element_offset.second - scroll_offset[ 1 ];
            case DROP:
            case PINCH:
            case SCROLL:
                e.position[ 0 ] -= element_offset.first  - scroll_offset[ 0 ];
                e.position[ 1 ] -= element_offset.second - scroll_offset[ 1 ];
                break;
            case KEYCOMMAND:
            case COMMAND:
            case TEXT:
                break;
            default:
                throw exception( "group::acceptEvent(): Unknown event type" );
                break;
            }
            
            return elements[ i ] -> acceptEvent( e );
        }
        else
            return false;
    }
    
    void group::updateScrollParams()
    {
        dpi::points v_top_left [] = { -scroll_offset[ 0 ], -scroll_offset[ 1 ] };
        dpi::points v_bot_right[] = { dimensions[ 0 ] - scroll_offset[ 0 ], dimensions[ 1 ] - scroll_offset[ 1 ] };
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< dpi::points, dpi::points > erp = elements[ i ] -> getRealPosition();
            std::pair< dpi::points, dpi::points > erd = elements[ i ] -> getRealDimensions();
            
            if( erp.first - scroll_offset[ 0 ] < v_top_left[ 0 ] )
                v_top_left[ 0 ] = erp.first - scroll_offset[ 0 ];
            if( erp.second - scroll_offset[ 1 ] < v_top_left[ 1 ] )
                v_top_left[ 1 ] = erp.second - scroll_offset[ 1 ];
            
            if( erp.first + erd.first - scroll_offset[ 0 ] > v_bot_right[ 0 ] )
                v_bot_right[ 0 ] = erp.first + erd.first - scroll_offset[ 0 ];
            if( erp.second + erd.second - scroll_offset[ 1 ] > v_bot_right[ 1 ] )
                v_bot_right[ 1 ] = erp.second + erd.second - scroll_offset[ 1 ];
        }
        
        scroll_limits[ 0 ] = ( v_bot_right[ 0 ] - v_top_left[ 0 ] ) - dimensions[ 0 ];
        scroll_limits[ 1 ] = ( v_bot_right[ 1 ] - v_top_left[ 1 ] ) - dimensions[ 1 ];
        
        if( scroll_limits[ 0 ] < 0 )
            scroll_limits[ 0 ] = 0;
        if( scroll_limits[ 1 ] < 0 )
            scroll_limits[ 1 ] = 0;
        
        clampScroll();
    }
    void group::clampScroll()
    {
        if( scroll_offset[ 0 ] > scroll_limits[ 0 ] )
            scroll_offset[ 0 ] = scroll_limits[ 0 ];
        else
            if( scroll_offset[ 0 ] < 0 )
                scroll_offset[ 0 ] = 0;
            
        if( scroll_offset[ 1 ] > scroll_limits[ 1 ] )
            scroll_offset[ 1 ] = scroll_limits[ 1 ];
        else
            if( scroll_offset[ 1 ] < 0 )
                scroll_offset[ 1 ] = 0;
    }
}


