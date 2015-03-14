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

// TODO: Bring this up to current style standards

namespace jade
{
    class scrollset : public gui_element
    {
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
        int          slider_pos  [ 2 ];                                         // horizontal, vertical
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
        jb_platform_idevid_t captured_dev;
        int capture_start[ 3 ];
        
        bool bars_always_visible;
        
        void arrangeBars();
        
        void init();
    public:
        scrollset( window* parent,
                   int x,
                   int y,
                   unsigned int w,
                   unsigned int h,
                   const std::shared_ptr< scrollable >& c );
        scrollset( window* parent,
                   int x,
                   int y,
                   const std::shared_ptr< scrollable >& c );                    // Inherits width & height from contents
        ~scrollset();
        
        void setParentWindow( window* );
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        void setBarsAlwaysVisible( bool v );
        bool getBarsAlwaysVisible();
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


