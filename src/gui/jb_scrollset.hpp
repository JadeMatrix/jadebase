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

#include "jb_element.hpp"
#include "jb_scrollable.hpp"
#include "../utility/jb_sharedpointer.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class scrollset : public gui_element
    {
    protected:
        shared_ptr< scrollable > contents;
        
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
    public:
        scrollset( window* parent,
                   int x,
                   int y,
                   unsigned int w,
                   unsigned int h,
                   shared_ptr< scrollable >& c );                               // Using shared_ptr<>& here is safe as it is copied on storage
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


