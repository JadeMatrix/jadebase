#ifndef BQT_GUI_BUTTON_HPP
#define BQT_GUI_BUTTON_HPP

/* 
 * bqt_gui_button.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_element.hpp"

/******************************************************************************//******************************************************************************/

#define BUTTON_MIN_WIDTH  12
#define BUTTON_MIN_HEIGHT 14

namespace bqt
{
    enum button_state
    {
        OFF_UP,
        OFF_DOWN,
        ON_UP,
        ON_DOWN
    };
    
    class button : public gui_element
    {
    protected:
        button_state state;
    public:
        button( window& parent,
                int x = 0,
                int y = 0,
                unsigned int w = BUTTON_MIN_WIDTH,
                unsigned int h = BUTTON_MIN_HEIGHT );
        ~button();
        
        void setRealDimensions( unsigned int w, unsigned int h );
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


