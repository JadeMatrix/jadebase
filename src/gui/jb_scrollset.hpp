#ifndef JADEBASE_SCROLLSET_HPP
#define JADEBASE_SCROLLSET_HPP

/* 
 * jb_scrollset.hpp
 * 
 * About
 * 
 * If the scrollable passed to scrollset's constructor is a group, its event
 * fallthrough should be set to true so the scrollset gets scroll events that
 * aren't otherwise accepted by the group's contents.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <memory>

#include "jb_element.hpp"
#include "jb_scrollable.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class scrollset : public gui_element
    {
    public:
        scrollset( dpi::points,
                   dpi::points,
                   dpi::points,
                   dpi::points,
                   const std::shared_ptr< scrollable >& );
        scrollset( dpi::points,
                   dpi::points,
                   const std::shared_ptr< scrollable >& );                      // Inherits width & height from contents
        ~scrollset();
        
        void setRealPosition(   dpi::points, dpi::points );
        void setRealDimensions( dpi::points, dpi::points );
        
        void setBarsAlwaysVisible( bool );
        bool getBarsAlwaysVisible();
        
        bool acceptEvent( window_event& );
        
        void draw( window* );
        
        void clearDeviceAssociations();
        
    protected:
        std::shared_ptr< scrollable > contents;
        
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
        dpi::points  slider_pos  [ 2 ];                                         // horizontal, vertical
        dpi::points  slider_width[ 2 ];                                         // horizontal, vertical
        
        enum
        {
            NONE = 0,                                                           // Defined as 0 so we can use if( capturing )
            HORIZONTAL_BAR,
            VERTICAL_BAR,
            LEFT_BUTTON,
            RIGHT_BUTTON,
            TOP_BUTTON,
            BOTTOM_BUTTON,
            CORNER
        } capturing;
        jb_platform_idevid_t captured_dev;
        dpi::points capture_start[ 3 ];
        
        bool bars_always_visible;
        
        void arrangeBars();
        
        void init();
    };
}

/******************************************************************************//******************************************************************************/

#endif


