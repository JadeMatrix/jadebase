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
        group( window*,                                                         // Initial parent window (can be NULL)
               int,                                                             // Initial X position in pixels
               int,                                                             // Initial Y position in pixels
               unsigned int,                                                    // Initial width in pixels
               unsigned int );                                                  // Initial height in pixels
        
        void addElement( const std::shared_ptr< gui_element >& );
        void removeElement( const std::shared_ptr< gui_element >& );            // Assumes caller retains a shared_ptr
        
        void setDrawBackground( bool );                                         // Set whether the group has an opaque or transparent background
        bool getDrawBackground();
        
        void setEventFallthrough( bool t );                                     // Set whether the group accepts all events all its children don't
        bool getEventFallthrough();
        
        // CALLBACKS & EVENTS //////////////////////////////////////////////////
        
        void setShownCallback( const std::shared_ptr< callback >& );            // Setting these to empty shared_ptrs disables them
        void setHiddenCallback( const std::shared_ptr< callback >& );
        void setClosedCallback( const std::shared_ptr< callback >& );
        
        void shown();
        void hidden();
        void closed();
        
        // GUI_ELEMENT /////////////////////////////////////////////////////////
        
        void setParentWindow( window* );
        
        void setRealPosition( int, int );                                       // X, Y
        void setRealDimensions( unsigned int, unsigned int );                   // Width, height
        
        std::pair< int, int > getVisualPosition();
        std::pair< unsigned int, unsigned int > getVisualDimensions();
        
        bool acceptEvent( window_event& );
        
        void draw();
        
        // SCROLLABLE //////////////////////////////////////////////////////////
        
        void scrollPixels(    int,   int );                                     // X, Y
        void scrollPercent( float, float );                                     // X, Y
        
        void setScrollPixels(    int,   int );                                  // X, Y
        void setScrollPercent( float, float );                                  // X, Y
        
        std::pair<   int,   int > getScrollPixels();
        std::pair< float, float > getScrollPercent();
        
        bool hasScrollLimit();
        std::pair<   int,   int > getScrollLimitPixels();
        std::pair< float, float > getScrollLimitPercent();
        
    protected:
        std::vector< std::shared_ptr< gui_element > > elements;
        unsigned int internal_dims[ 2 ];                                        // Dimensions of the internal layout of the group
        
        bool draw_background;
        
        bool event_fallthrough;
        
        std::shared_ptr< callback > shown_callback;
        std::shared_ptr< callback > hidden_callback;
        std::shared_ptr< callback > closed_callback;
        
        int scroll_limits[ 2 ];                                                 // X min, X max, Y min, Y max
        int scroll_offset[ 2 ];
        
        bool acceptEvent_copy( window_event );                                  // Copies event first for modification
        
        void updateScrollParams();                                              // Utility, not thread-safe
    };
}

/******************************************************************************//******************************************************************************/

#endif


