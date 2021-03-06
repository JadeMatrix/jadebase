#ifndef JADEBASE_GROUP_HPP
#define JADEBASE_GROUP_HPP

/* 
 * jb_group.hpp
 * 
 * Scriptable, generic element groups
 * 
 * shown() and hidden() do not directly affect the group's rendering, rather
 * they are called by some parent when their view of the group changes.  These
 * are merely handles in case the group script relies on this information.
 * 
 * There is a potential design flaw in that a gui element currently capturing
 * device input will continue to do so even if any parent group(s) are hidden.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <memory>
#include <string>
#include <vector>

#include <lua.hpp>

#include "jb_element.hpp"
#include "jb_scrollable.hpp"
#include "../utility/jb_callback.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class group : public scrollable
    {
    public:
        group( dpi::points,                                                     // Initial X position in points
               dpi::points,                                                     // Initial Y position in points
               dpi::points,                                                     // Initial width in points
               dpi::points );                                                   // Initial height in points
        
        void addElement(    const std::shared_ptr< gui_element >& );
        void removeElement( const std::shared_ptr< gui_element >& );            // Assumes caller retains a shared_ptr
        
        void setDrawBackground( bool );                                         // Set whether the group has an opaque or transparent background
        bool getDrawBackground();
        
        void setStrokeFallthrough( bool );                                      // Set whether the group accepts all stroke events left unaccepted by children
        bool getStrokeFallthrough();
        
        // CALLBACKS & EVENTS //////////////////////////////////////////////////
        
        void setShownCallback(  const std::shared_ptr< callback >& );           // Setting these to empty shared_ptrs disables them
        void setHiddenCallback( const std::shared_ptr< callback >& );
        void setClosedCallback( const std::shared_ptr< callback >& );
        
        void shown();
        void hidden();
        void closed();
        
        // GUI_ELEMENT /////////////////////////////////////////////////////////
        
        void setRealPosition(   dpi::points, dpi::points );                     // X, Y
        void setRealDimensions( dpi::points, dpi::points );                     // Width, height
        
        std::pair< dpi::points, dpi::points > getVisualPosition();
        std::pair< dpi::points, dpi::points > getEventOffset();
        std::pair< dpi::points, dpi::points > getVisualDimensions();
        
        bool acceptEvent( window_event& );
        
        void draw( window* );
        
        void clearDeviceAssociations();
        
        // SCROLLABLE //////////////////////////////////////////////////////////
        
        // TODO: Possible change these to overloads on dpi::points vs. dpi::percent?
        
        void scrollPoints(  dpi::points,  dpi::points  );                       // X, Y
        void scrollPercent( dpi::percent, dpi::percent );                       // X, Y
        
        void setScrollPoints(  dpi::points,  dpi::points  );                    // X, Y
        void setScrollPercent( dpi::percent, dpi::percent );                    // X, Y
        
        std::pair< dpi::points,  dpi::points  > getScrollPoints();
        std::pair< dpi::percent, dpi::percent > getScrollPercent();
        
        bool hasScrollLimit();
        std::pair< dpi::points,  dpi::points  > getScrollLimitPoints();
        std::pair< dpi::percent, dpi::percent > getScrollLimitPercent();
        
    protected:
        std::vector< std::shared_ptr< gui_element > > elements;
        
        bool draw_background;
        
        bool stroke_fallthrough;
        
        std::shared_ptr< callback > shown_callback;
        std::shared_ptr< callback > hidden_callback;
        std::shared_ptr< callback > closed_callback;
        
        dpi::points scroll_limits[ 2 ];                                         // Max scrollable points in each direction
        dpi::points scroll_offset[ 2 ];                                         // Subtractive offset in each direction (usually positive)
        
        bool sendEventToChild( int, window_event );                             // Utility; copies event first for modification; not thread-safe
        
        void updateScrollParams();                                              // Utility; not thread-safe
        void clampScroll();                                                     // Utility; not thread-safe
    };
}

/******************************************************************************//******************************************************************************/

#endif


