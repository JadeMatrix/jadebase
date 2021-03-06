#ifndef JADEBASE_SCROLLABLE_HPP
#define JADEBASE_SCROLLABLE_HPP

/* 
 * jb_scrollable.hpp
 * 
 * Abstract class for scrollable gui elements
 *
 * The simplest way to implement scrollable areas would be with a group - offset
 * the group's position & mask during rendering.  However, we want scrollsets
 * to be content-agnostic.  They must be usable not only for areas containing
 * other GUI elements but also for such things as drawing canvasses and text
 * areas.  Rendering these in their entirety and simply masking the result would
 * potentially be drastic, especially if some scrollable element has no hard
 * logical extents.  The scrolled element must know how much of itself and at
 * what offset it is being drawn.  jade::scrollable is an interface that gives
 * jade::scrollset just as much information it needs about its contents to give
 * the user scroll feedback and interactivity, leaving the implementation
 * entirely up to the inheriting class.
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

#include "jb_element.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class scrollable : public gui_element
    {
    public:
        scrollable( dpi::points x,
                    dpi::points y,
                    dpi::points w, 
                    dpi::points h ) : gui_element( x, y, w, h ) {}
        virtual ~scrollable() {}
        
        virtual void setRealDimensions( dpi::points, dpi::points ) = 0;         // Width, height
        
        virtual void scrollPoints(  dpi::points,  dpi::points  ) = 0;           // X, Y
        virtual void scrollPercent( dpi::percent, dpi::percent ) = 0;           // X, Y
        
        virtual void setScrollPoints(  dpi::points,  dpi::points  ) = 0;        // X, Y
        virtual void setScrollPercent( dpi::percent, dpi::percent ) = 0;        // X, Y
        
        virtual std::pair< dpi::points,  dpi::points  > getScrollPoints()  = 0;
        virtual std::pair< dpi::percent, dpi::percent > getScrollPercent() = 0; // ... as a +/- % of real dimensions
        
        virtual bool hasScrollLimit() = 0;
        virtual std::pair< dpi::points,  dpi::points  > getScrollLimitPoints()  = 0;
        virtual std::pair< dpi::percent, dpi::percent > getScrollLimitPercent() = 0;    // ... as a +/- % of real dimensions
    };
}

/******************************************************************************//******************************************************************************/

#endif


