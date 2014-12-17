#ifndef BQT_SCROLLABLE_HPP
#define BQT_SCROLLABLE_HPP

/* 
 * bqt_scrollable.hpp
 * 
 * Abstract class for scrollable gui elements
 * 
 * scroll***(): Scroll a relative amount
 * setScroll***(): Scroll an absolute amount
 * getScroll***(): Get current scroll amount
 * getScrollLimit***(): Get minimum & maximum x & y scroll limits
 * 
 * Classes that inherit from scrollable are responsible for clamping values from
 * scroll***() calls.
 * 
 * Scrolling with negative values will result in a negative offset, and vice
 * versa.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
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
        
        virtual void scrollPixels(    int x,   int y ) = 0;
        virtual void scrollPercent( float x, float y ) = 0;
        
        virtual void setScrollPixels(   int x,    int y ) = 0;
        virtual void setScrollPercent( float x, float y ) = 0;
        
        virtual std::pair<   int,   int > getScrollPixels()  = 0;
        virtual std::pair< float, float > getScrollPercent() = 0;               // ... as a +/- % of real dimensions
        
        virtual bool hasScrollLimit() = 0;
        virtual std::pair<    int,  int > getScrollLimitPixels()  = 0;
        virtual std::pair< float, float > getScrollLimitPercent() = 0;          // ... as a +/- % of real dimensions
    };
}

/******************************************************************************//******************************************************************************/

#endif


