#ifndef BQT_GUI_SCROLLSET_HPP
#define BQT_GUI_SCROLLSET_HPP

/* 
 * bqt_gui_scrollset.hpp
 * 
 * About
 * 
 * If the scrollable passed to scrollset's constructor is a group, its event
 * fallthrough should be set to true so the scrollset gets scroll events that
 * aren't otherwise accepted by the group's contents.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_element.hpp"
#include "bqt_scrollable.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class scrollset : public gui_element
    {
    protected:
        scrollable* contents;
    public:
        scrollset( window& parent,
                   int x,
                   int y,
                   unsigned int w,
                   unsigned int h,
                   scrollable* c = NULL );
        ~scrollset();
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


