/* 
 * bqt_gui_group.cpp
 * 
 * Implements bqt_gui_group.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_group.hpp"

#include "../bqt_exception.hpp"
#include "../bqt_gl.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool group::acceptEvent_copy( window_event e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        bool no_position = false;
        
        std::pair< int, int > element_position;
        std::pair< unsigned int, unsigned int > element_dimensions;
        int e_position[ 2 ];
        
        e.offset[ 0 ] += position[ 0 ];
        e.offset[ 1 ] += position[ 1 ];
        
        switch( e.type )
        {
        case STROKE:
            e_position[ 0 ] = e.stroke.position[ 0 ];
            e_position[ 1 ] = e.stroke.position[ 1 ];
            break;
        case DROP:
            e_position[ 0 ] = e.drop.position[ 0 ];
            e_position[ 1 ] = e.drop.position[ 1 ];
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            no_position = true;
            break;
        case PINCH:
            e_position[ 0 ] = e.pinch.position[ 0 ];
            e_position[ 1 ] = e.pinch.position[ 1 ];
            break;
        case SCROLL:
            e_position[ 0 ] = e.scroll.position[ 0 ];
            e_position[ 1 ] = e.scroll.position[ 1 ];
            break;
        default:
            throw exception( "group::acceptEvent(): Unknown event type" );
            break;
        }
        
        e_position[ 0 ] -= e.offset[ 0 ];
        e_position[ 1 ] -= e.offset[ 1 ];
        
        for( int i = elements.size() - 1; i >= 0; -- i )                        // Iterate newest (topmost) first
        {
            if( no_position )
            {
                if( elements[ i ] -> acceptEvent( e ) )
                    return true;
            }
            else
            {
                element_position   = elements[ i ] -> getVisualPosition();
                element_dimensions = elements[ i ] -> getVisualDimensions();
                
                if( pointInsideRect( e.stroke.prev_pos[ 0 ] - e.offset[ 0 ],
                                     e.stroke.prev_pos[ 1 ] - e.offset[ 1 ],
                                     element_position.first,
                                     element_position.second,
                                     element_dimensions.first,
                                     element_dimensions.second )
                    || pointInsideRect( e_position[ 0 ],
                                        e_position[ 1 ],
                                        element_position.first,
                                        element_position.second,
                                        element_dimensions.first,
                                        element_dimensions.second ) )
                {
                    if( elements[ i ] -> acceptEvent( e ) )
                        return true;
                }
            }
        }
        
        if( event_fallthrough )
            return false;
        else
            return ( no_position
                     || pointInsideRect( e_position[ 0 ],
                                         e_position[ 1 ],
                                         0,
                                         0,
                                         dimensions[ 0 ],
                                         dimensions[ 1 ] ) );
            // return true;
    }
    
    group::group( window& parent,
                  int x,
                  int y,
                  unsigned int w,
                  unsigned int h,
                  std::string f ) : scrollable( parent, x, y, w, h )
    {
        event_fallthrough = false;
        
        scroll_limits[ 0 ] = -256;
        scroll_limits[ 1 ] = 256;
        scroll_limits[ 2 ] = -256;
        scroll_limits[ 3 ] = 256;
        
        // scroll_limits[ 0 ] = 0;
        // scroll_limits[ 1 ] = 0;
        // scroll_limits[ 2 ] = 0;
        // scroll_limits[ 3 ] = 0;
        
        scroll_offset[ 0 ] = 0;
        scroll_offset[ 1 ] = 0;
        
        if( f != "" )
        {
            // initialize lua_state
        }
    }
    group::~group()
    {
        // if lua_state is initialized
        //     destroy lua_state
    }
    
    void group::addElement( gui_element* e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        elements.push_back( e );
    }
    void group::removeElement( gui_element* e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        for( std::vector< gui_element* >::iterator iter = elements.begin();
             iter != elements.end();
             ++iter )
        {
            if( *iter == e )
            {
                elements.erase( iter );
                
                parent.requestRedraw();
                
                return;
            }
        }
        
        throw exception( "group::removeElement(): No such element" );
    }
    
    void group::shown()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // inform lua_state
    }
    void group::hidden()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // inform lua_state
    }
    
    void group::close()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // inform lua_state
    }
    
    bool group::getEventFallthrough()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return event_fallthrough;
    }
    void group::setEventFallthrough( bool t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        event_fallthrough = t;
    }
    
    // GUI_ELEMENT /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void group::setRealPosition( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        // inform lua_state
        
        parent.requestRedraw();
    }
    void group::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        // inform lua_state
        
        parent.requestRedraw();
    }
    
    std::pair< int, int > group::getVisualPosition()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::pair< int, int > v_position = getRealPosition();
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< int, int > evp = elements[ i ] -> getVisualPosition();
            
            if( evp.first < v_position.first )
                v_position.first = evp.first;
            
            if( evp.second < v_position.second )
                v_position.second = evp.second;
        }
        
        return v_position;
    }
    std::pair< unsigned int, unsigned int > group::getVisualDimensions()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // std::pair< int, int > v_position = getVisualPosition();
        std::pair< unsigned int, unsigned int > v_dimensions = getRealDimensions();
        
        // if( v_position.first < position[ 0 ] )                                  // Always guaranteed to be <= from getVisualPosition()
        //     v_dimensions.first += position[ 0 ] - v_position.first;
        // if( v_position.second < position[ 1 ] )
        //     v_dimensions.second += position[ 1 ] - v_position.second;
        
        // for( int i = 0; i < elements.size(); ++i )
        // {
        //     std::pair< int, int > evp = elements[ i ] -> getVisualPosition();
        //     std::pair< unsigned int, unsigned int > evd = elements[ i ] -> getVisualPosition();
            
        //     if( evp.first + evd.first > v_position.first + v_dimensions.first )
        //         v_dimensions.first = evd.first;
            
        //     if( evp.second + evd.second > v_position.second + v_dimensions.second )
        //         v_dimensions.second = evd.second;
        // }
        
        return v_dimensions;
    }
    
    bool group::acceptEvent( window_event& e )
    {
        return acceptEvent_copy( e );                                           // Easy way of changing offsets without editing original
    }
    
    void group::draw()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
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
            
            for( int i = 0; i < elements.size(); ++i )
            {
                ff::write( bqt_out, "Masking draw area ", position[ 0 ], ", ", position[ 1 ], ": ", dimensions[ 0 ], "x", dimensions[ 1 ], "\n" );
                addDrawMask( 0, 0, dimensions[ 0 ], dimensions[ 1 ] );          // Do it for every element, as they might erase the mask
                elements[ i ] -> draw();
            }
            
            clearDrawMasks();
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
    
    // SCROLLABLE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void group::scrollPixels( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( scroll_limits[ 0 ] > scroll_limits[ 1 ]
            || scroll_limits[ 2 ] > scroll_limits[ 3 ] )
        {
            throw exception( "group::scrollPixels(): Some min limit above max limit" );
        }
        
        if( scroll_offset[ 0 ] + x < scroll_limits[ 0 ] )
            x = scroll_limits[ 0 ] - scroll_offset[ 0 ];
        else
            if( scroll_offset[ 0 ] + x > scroll_limits[ 1 ] )
                x = scroll_limits[ 1 ] - scroll_offset[ 0 ];
        
        if( scroll_offset[ 1 ] + y < scroll_limits[ 2 ] )
            y = scroll_limits[ 2 ] - scroll_offset[ 1 ];
        else
            if( y > scroll_limits[ 3 ] )
                y = scroll_limits[ 3 ] - scroll_offset[ 1 ];
        
        ff::write( bqt_out,
                   "Group scrolling by ",
                   x,
                   ", ",
                   y,
                   "\n" );
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< int, int > old_pos = elements[ i ] -> getRealPosition();
            elements[ i ] -> setRealPosition( old_pos.first + x, old_pos.second + y );
        }
        
        parent.requestRedraw();
    }
    void group::scrollPercent( float x, int y )
    {
        scrollPixels( x * dimensions[ 0 ], y * dimensions[ 1 ] );
    }
    
    std::pair< int, int > group::getScrollPixels()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< int, int >( scroll_offset[ 0 ], scroll_offset[ 1 ] );
    }
    std::pair< float, float > group::getScrollPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< float, float >( ( float )scroll_offset[ 0 ], ( float )scroll_offset[ 1 ] );
    }
    
    bool group::hasScrollLimit()
    {
        return true;
    }
    limit_pixels group::getScrollLimitPixels()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        limit_pixels l;
        
        l.first.first = scroll_limits[ 0 ];
        l.first.second = scroll_limits[ 1 ];
        l.second.first = scroll_limits[ 2 ];
        l.second.second = scroll_limits[ 3 ];
        
        return l;
    }
    limit_percent group::getScrollLimitPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        limit_percent l;
        
        l.first.first = ( float )scroll_limits[ 0 ];
        l.first.second = ( float )scroll_limits[ 1 ];
        l.second.first = ( float )scroll_limits[ 2 ];
        l.second.second = ( float )scroll_limits[ 3 ];
        
        return l;
    }
}


