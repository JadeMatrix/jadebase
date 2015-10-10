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
        
        void setEventFallthrough( bool );                                       // Set whether the group accepts all events all its children don't
        bool getEventFallthrough();
        
        // CALLBACKS & EVENTS //////////////////////////////////////////////////
        
        void setShownCallback(  const std::shared_ptr< callback >& );           // Setting these to empty shared_ptrs disables them
        void setHiddenCallback( const std::shared_ptr< callback >& );
        void setClosedCallback( const std::shared_ptr< callback >& );
        
        void shown();
        void hidden();
        void closed();
        
        // GUI_ELEMENT /////////////////////////////////////////////////////////
        
        void setParentElement( gui_element* );
        
        void setRealPosition(   dpi::points, dpi::points );                     // X, Y
        void setRealDimensions( dpi::points, dpi::points );                     // Width, height
        
        std::pair< dpi::points, dpi::points > getVisualPosition();
        std::pair< dpi::points, dpi::points > getVisualDimensions();
        
        bool acceptEvent( window_event& );
        
        void draw( window* );
        
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
        dpi::points internal_dims[ 2 ];                                         // Dimensions of the internal layout of the group
        
        bool draw_background;
        
        bool event_fallthrough;
        
        std::shared_ptr< callback > shown_callback;
        std::shared_ptr< callback > hidden_callback;
        std::shared_ptr< callback > closed_callback;
        
        dpi::points scroll_limits[ 2 ];                                         // X min, X max, Y min, Y max
        dpi::points scroll_offset[ 2 ];
        
        bool acceptEvent_copy( window_event );                                  // Copies event first for modification
        
        void updateScrollParams();                                              // Utility, not thread-safe
    };
}

/******************************************************************************//******************************************************************************/

#endif


