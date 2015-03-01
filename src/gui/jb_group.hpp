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

#include <string>
#include <vector>

#include <lua.hpp>

#include "jb_element.hpp"
#include "jb_scrollable.hpp"
#include "../utility/jb_container.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class group : public scrollable
    {
    protected:
        std::vector< container< gui_element > > elements;
        unsigned int internal_dims[ 2 ];                                        // Dimensions of the internal layout of the group
        
        bool draw_background;
        
        bool event_fallthrough;
        
        gui_callback* shown_callback;
        gui_callback* hidden_callback;
        gui_callback* closed_callback;
        
        int scroll_limits[ 2 ];                                                 // X min, X max, Y min, Y max
        int scroll_offset[ 2 ];
        
        bool acceptEvent_copy( window_event e );                                // Copies event first for modification
        
        void updateScrollParams();                                              // Utility, not thread-safe
    public:
        group( window* parent,
               int x,
               int y,
               unsigned int w,
               unsigned int h );
        ~group();
        
        void addElement( container< gui_element >& e );                         // Safe to use container<>& here as it is copied during storage
        void removeElement( container< gui_element >& e );                      // Safe to use container<>& here as it is only used locally to the function
        
        void drawBackground( bool );
        
        bool getEventFallthrough();
        void setEventFallthrough( bool t );
        
        // CALLBACKS & EVENTS //////////////////////////////////////////////////
        
        gui_callback* setShownCallback( gui_callback* );                        // All callback pointers belong to the group after setting them, unless changed
        gui_callback* setHiddenCallback( gui_callback* );                       // or set to NULL.  They return the previous callback pointer.
        gui_callback* setClosedCallback( gui_callback* );
        
        void shown();
        void hidden();
        void closed();
        
        // GUI_ELEMENT /////////////////////////////////////////////////////////
        
        void setParentWindow( window* );
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        std::pair< int, int > getVisualPosition();
        std::pair< unsigned int, unsigned int > getVisualDimensions();
        
        bool acceptEvent( window_event& e );
        
        void draw();
        
        // SCROLLABLE //////////////////////////////////////////////////////////
        
        void scrollPixels(    int x,   int y );
        void scrollPercent( float x, float y );
        
        void setScrollPixels(    int x,   int y );
        void setScrollPercent( float x, float y );
        
        std::pair<   int,   int > getScrollPixels();
        std::pair< float, float > getScrollPercent();
        
        bool hasScrollLimit();
        std::pair<   int,   int > getScrollLimitPixels();
        std::pair< float, float > getScrollLimitPercent();
    };
}

/******************************************************************************//******************************************************************************/

#endif


