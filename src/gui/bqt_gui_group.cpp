/* 
 * bqt_gui_group.cpp
 * 
 * Implements bqt_gui_group.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_group.hpp"

#include "../bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool group::acceptEvent_copy( window_event e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
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
                
                if( ( e.type == STROKE
                      && pointInsideRect( e.stroke.prev_pos[ 0 ] - e.offset[ 0 ],
                                          e.stroke.prev_pos[ 1 ] - e.offset[ 1 ],
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
                        return true;
                }
            }
        }
        
        return !event_fallthrough;
    }
    
    group::group( window& parent,
                  int x,
                  int y,
                  unsigned int w,
                  unsigned int h,
                  std::string f ) : gui_element( parent, x, y, w, h )
    {
        event_fallthrough = false;
        
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
    
    void group::setRealPosition( int x, int y )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        // inform lua_state
        
        parent.requestRedraw();
    }
    void group::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        // inform lua_state
        
        parent.requestRedraw();
    }
    
    std::pair< int, int > group::getVisualPosition()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
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
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        std::pair< int, int > v_position = getVisualPosition();
        std::pair< unsigned int, unsigned int > v_dimensions = getRealDimensions();
        
        if( v_position.first < position[ 0 ] )                                  // Always guaranteed to be <= from getVisualPosition()
            v_dimensions.first += position[ 0 ] - v_position.first;
        if( v_position.second < position[ 1 ] )
            v_dimensions.second += position[ 1 ] - v_position.second;
        
        for( int i = 0; i < elements.size(); ++i )
        {
            std::pair< int, int > evp = elements[ i ] -> getVisualPosition();
            std::pair< unsigned int, unsigned int > evd = elements[ i ] -> getVisualPosition();
            
            if( evp.first + evd.first > v_position.first + v_dimensions.first )
                v_dimensions.first = evd.first;
            
            if( evp.second + evd.second > v_position.second + v_dimensions.second )
                v_dimensions.second = evd.second;
        }
        
        return v_dimensions;
    }
    
    void group::addElement( gui_element* e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        elements.push_back( e );
    }
    void group::removeElement( gui_element* e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
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
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        // inform lua_state
    }
    void group::hidden()
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        // inform lua_state
    }
    
    void group::close()
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        // inform lua_state
    }
    
    bool group::acceptEvent( window_event& e )
    {
        return acceptEvent_copy( e );
    }
    
    void group::draw()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
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
                elements[ i ] -> draw();
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


