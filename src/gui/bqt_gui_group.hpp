#ifndef BQT_GUI_GROUP_HPP
#define BQT_GUI_GROUP_HPP

/* 
 * bqt_gui_group.hpp
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

#include <vector>
#include <string>

#include <lua.hpp>

#include "bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class group : public gui_element
    {
    protected:
        std::vector< gui_element* > elements;
        
        lua_State* lua_state;
        
        bool acceptEvent_copy( window_event e );                                // Copies event first for modification
    public:
        group( window& parent,
               int x,
               int y,
               unsigned int w,
               unsigned int h,
               std::string f = "" );
        ~group();
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        std::pair< int, int > getVisualPosition();
        std::pair< unsigned int, unsigned int > getVisualDimensions();
        
        void addElement( gui_element* e );
        void removeElement( gui_element* e );
        
        void shown();
        void hidden();
        
        void close();
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


