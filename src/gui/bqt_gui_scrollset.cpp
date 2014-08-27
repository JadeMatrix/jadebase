/* 
 * bqt_gui_scrollset.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_scrollset.hpp"

#include "bqt_gui_resource.hpp"
#include "bqt_gui_group.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    
}

/******************************************************************************//******************************************************************************/

#define SCROLLBAR_WIDTH 12

namespace bqt
{
    scrollset::scrollset( window& parent,
                          int x,
                          int y,
                          unsigned int w,
                          unsigned int h,
                          scrollable* c ) : gui_element( parent, x, y, w, h )
    {
        if( c == NULL )
        {
            group* g = new group( parent,
                                  position[ 0 ],
                                  position[ 1 ],
                                  dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                  dimensions[ 1 ] - SCROLLBAR_WIDTH );
            g -> setEventFallthrough( true );
            
            contents = g;
        }
        else
        {
            c -> setRealPosition( position[ 0 ], position[ 1 ] );
            c -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                    dimensions[ 1 ] - SCROLLBAR_WIDTH );
            
            contents = c;
        }
    }
    scrollset::~scrollset()
    {
        delete contents;
    }
    
    void scrollset::setRealPosition( int x, int y )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                       dimensions[ 1 ] - SCROLLBAR_WIDTH );
        
        parent.requestRedraw();
    }
    void scrollset::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                       dimensions[ 1 ] - SCROLLBAR_WIDTH );
        
        parent.requestRedraw();
    }
    
    bool scrollset::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        
        
        return false;
    }
    
    void scrollset::draw()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        
    }
}


