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
        
        enum button_state
        {
            UP,
            DOWN,
            EVIL,
            EVIL_DOWN,
            DISABLED
        };
        
        button_state horz_state[ 2 ];
        button_state vert_state[ 2 ];
        button_state corner_state;
        
        button_state slider_state[ 2 ];                                         // horizontal, vertical
        int          slider_pos[ 2 ];                                           // horizontal, vertical
        unsigned int slider_width[ 2 ];                                         // horizontal, vertical
        
        enum
        {
            NONE = 0,
            HORIZONTAL_BAR,
            VERTICAL_BAR,
            LEFT_BUTTON,
            RIGHT_BUTTON,
            TOP_BUTTON,
            BOTTOM_BUTTON,
            CORNER
        } capturing;
        bqt_platform_idevid_t captured_dev;
        int capture_start[ 3 ];
        
        void arrangeBars();
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


