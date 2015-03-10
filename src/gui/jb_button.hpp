#ifndef JADEBASE_BUTTON_HPP
#define JADEBASE_BUTTON_HPP

/* 
 * jb_button.hpp
 * 
 * GUI element class for a basic button
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <memory>

#include "jb_element.hpp"
#include "jb_resource.hpp"
#include "../utility/jb_callback.hpp"

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
        
        std::shared_ptr< callback > toggle_on_callback;
        std::shared_ptr< callback > toggle_off_callback;
        
        std::shared_ptr< gui_resource > contents;
        resource_align contents_align;
        
        void setState( button_state );                                          // Internal utility function, not thread-safe
    public:
        button( window* parent,
                int x,
                int y,
                unsigned int w = BUTTON_MIN_WIDTH,
                unsigned int h = BUTTON_MIN_HEIGHT );
        
        void setToggleOnCallback( const std::shared_ptr< callback >& );
        void setToggleOffCallback( const std::shared_ptr< callback >& );
        
        void setRealDimensions( unsigned int w, unsigned int h );
        
        void setContents( const std::shared_ptr< gui_resource >& c,             // c - contents
                          resource_align a,                                     // a - alignment
                          bool r = false );                                     // r - resize button to match contents
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


