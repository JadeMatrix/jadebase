#ifndef JADEBASE_BUTTON_HPP
#define JADEBASE_BUTTON_HPP

/* 
 * jb_button.hpp
 * 
 * GUI element class for a basic button
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_element.hpp"
#include "jb_resource.hpp"

/******************************************************************************//******************************************************************************/

#define BUTTON_MIN_WIDTH  12
#define BUTTON_MIN_HEIGHT 14

namespace jade
{
    enum button_state
    {
        OFF_UP,
        OFF_DOWN,
        ON_UP,
        ON_DOWN
    };
    
    enum resource_align
    {
        TOP_LEFT,    TOP_CENTER,    TOP_RIGHT,
        CENTER_LEFT, CENTER_CENTER, CENTER_RIGHT,
        BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT
    };
    
    class button : public gui_element
    {
    protected:
        button_state state;
        jb_platform_idevid_t captured_dev;
        
        gui_resource* contents;
        bool contents_owner;
        resource_align contents_align;
    public:
        button( window* parent,
                int x = 0,
                int y = 0,
                unsigned int w = BUTTON_MIN_WIDTH,
                unsigned int h = BUTTON_MIN_HEIGHT );
        ~button();
        
        void setRealDimensions( unsigned int w, unsigned int h );
        
        void setContents( gui_resource* c,                                      // c - contents, NULL unsets contents
                          resource_align a,                                     // a - alignment
                          bool r = false,                                       // r - resize button to match contents
                          bool o = true );                                      // o - button owns contents & will delete on destruction
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


