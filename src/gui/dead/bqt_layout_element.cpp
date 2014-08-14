/* 
 * bqt_layout_element.cpp
 * 
 * Implements base class layout_element from bqt_layout_element.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_layout_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    layout_element::layout_element( layout& parent,
                                    int x,
                                    int y,
                                    unsigned int w,
                                    unsigned int h ) : parent( parent )
    {
        position[ 0 ] = x;
        position[ 1 ] = y;
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        event_fallthrough = true;
    }
    
    void layout_element::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    std::pair< unsigned int, unsigned int > layout_element::getRealDimensions()
    {
        scoped_lock slock( element_mutex );
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< unsigned int, unsigned int > layout_element::getVisualDimensions()
    {
        return getRealDimensions();                                             // Defaults to the same thing as getRealDimensions()
    }
    
    void layout_element::setPosition( int x, int y )
    {
        scoped_lock slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
    }
    std::pair< int, int > layout_element::getPosition()
    {
        scoped_lock slock( element_mutex );
        
        return std::pair< unsigned int, unsigned int >( position[ 0 ], position[ 1 ] );
    }
    
    void layout_element::setEventFallthrough( bool f )
    {
        scoped_lock slock( element_mutex );
        
        event_fallthrough = f;
    }
    bool layout_element::getEventFallthrough()
    {
        scoped_lock slock( element_mutex );
        
        return event_fallthrough;
    }
}


