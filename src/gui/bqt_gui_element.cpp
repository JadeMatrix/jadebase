/* 
 * bqt_gui_element.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void gui_element::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    
    gui_element::gui_element( window& parent,
                 int x,
                 int y,
                 unsigned int w,
                 unsigned int h ) : parent( parent )
    {
        setPosition( x, y );
        setRealDimensions( w, h );
        
        event_fallthrough = true;
    }
    gui_element::~gui_element()
    {
        /* Empty */
    }
    
    void gui_element::setPosition( int x, int y )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
    }
    std::pair< int, int > gui_element::getPosition()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    
    std::pair< unsigned int, unsigned int > gui_element::getRealDimensions()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< unsigned int, unsigned int > gui_element::getVisualDimensions()
    {
        return getRealDimensions();
    }
    
    void gui_element::setEventFallthrough( bool f )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        event_fallthrough = f;
    }
    bool gui_element::getEventFallthrough()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        return event_fallthrough;
    }
}


