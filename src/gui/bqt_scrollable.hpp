#ifndef BQT_SCROLLABLE_HPP
#define BQT_SCROLLABLE_HPP

/* 
 * bqt_scrollable.hpp
 * 
 * Abstract class for scrollable gui elements
 * 
 * scroll***(): Scroll an amount
 * getScroll***(): Get current scroll amount
 * getScrollLimit***(): Get minimum & maximum x & y scroll limits
 * 
 * Classes that inherit from scrollable are responsible for clamping values from
 * scroll***() calls.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    typedef std::pair< std::pair< int, int >, std::pair< int, int > > limit_pixels;
    typedef std::pair< std::pair< float, float >, std::pair< float, float > > limit_percent;
    
    class scrollable : public gui_element
    {
    public:
        scrollable( window& parent,
                    int x,
                    int y,
                    unsigned int w, 
                    unsigned int h ) : gui_element( parent, x, y, w, h ) {}
        virtual ~scrollable() {}
        
        virtual void setRealDimensions( unsigned int w, unsigned int h ) = 0;
        
        virtual void scrollPixels( int x, int y ) = 0;
        virtual void scrollPercent( float x, int y ) = 0;
        
        virtual std::pair< int, int > getScrollPixels() = 0;
        virtual std::pair< float, float > getScrollPercent() = 0;
        
        virtual bool hasScrollLimit() = 0;
        virtual limit_pixels getScrollLimitPixels() = 0;
        virtual limit_percent getScrollLimitPercent() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


