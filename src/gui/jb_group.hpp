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
        
        bool acceptEvent_copy( window_event e );                                // Copies event first for modification
        
        void updateScrollParams();                                              // Utility, not thread-safe
    public:
        group( window* parent,
               int x,
               int y,
               unsigned int w,
               unsigned int h );
        
        void addElement( const std::shared_ptr< gui_element >& e );
        void removeElement( const std::shared_ptr< gui_element >& e );
        
        void drawBackground( bool );
        
        bool getEventFallthrough();
        void setEventFallthrough( bool t );
        
        // CALLBACKS & EVENTS //////////////////////////////////////////////////
        
        void setShownCallback( const std::shared_ptr< callback >& );
        void setHiddenCallback( const std::shared_ptr< callback >& );
        void setClosedCallback( const std::shared_ptr< callback >& );
        
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


