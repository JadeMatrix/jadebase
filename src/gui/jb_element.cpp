/* 
 * jb_element.cpp
 * 
 * Implements jade::gui_element class
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_element.hpp"

#include "../windowsys/jb_window.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void gui_element::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    gui_element::gui_element( window* parent,
                 int x,
                 int y,
                 unsigned int w,
                 unsigned int h ) : parent( parent )
    {
        setRealPosition( x, y );
        setRealDimensions( w, h );
    }
    gui_element::~gui_element()
    {
        /* Empty */
    }
    
    window* gui_element::getParentWindow()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return parent;
    }
    void gui_element::setParentWindow( window* p )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        parent = p;
        
        if( parent != NULL )
            parent -> requestRedraw();                                          // Babby's first redraw request
    }
    
    void gui_element::setRealPosition( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    std::pair< int, int > gui_element::getRealPosition()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    std::pair< int, int > gui_element::getVisualPosition()
    {
        return getRealPosition();
    }
    
    std::pair< unsigned int, unsigned int > gui_element::getRealDimensions()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< unsigned int, unsigned int > gui_element::getVisualDimensions()
    {
        return getRealDimensions();
    }
}


