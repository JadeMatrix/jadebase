#ifndef bqt_gui_button_HPP
#define bqt_gui_button_HPP

/* 
 * bqt_gui_button.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_layout_element.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class button : public layout_element
    {
    protected:
        enum
        {
            BLANK,
            STRING,
            IMAGE
        } type;
        
        struct rsrc_group
        {
            gui_resource* top_left;
            gui_resource* top_center;
            gui_resource* top_right;
            gui_resource* center_left;
            gui_resource* center_center;
            gui_resource* center_right;
            gui_resource* bottom_left;
            gui_resource* bottom_center;
            gui_resource* bottom_right;
        };
        
        rsrc_group off_up;
        rsrc_group off_down;
        rsrc_group on_up;
        rsrc_group on_down;
        
        std::string str_contents;
        //??? img_contents;
        
        bool auto_resize;
        
        bool togglable;
        enum
        {
            OFF_UP,
            OFF_DOWN,
            ON_UP,
            ON_DOWN
        } state;
    public:
        button( layout& parent,
                int x,
                int y,
                unsigned int w,
                unsigned int h );
        ~button();
        
        std::pair< unsigned int, unsigned int > getRealDimensions();
        // virtual std::pair< unsigned int, unsigned int > getVisualDimensions();
        
        virtual bool acceptEvent( window_event&e );
        
        virtual void draw();
        
        /* button-specific ****************************************************//******************************************************************************/
        
        void setText ( std::string t, bool c = true );                          // Sets string contents to t; if c switches type to string
        // void setImage( ???? i, bool c = true );                                 // Sets image contents to i; if c switches type to image
        void setImage( std::string f, bool c = true );                          // Loads image file f & sets contents; if c switches type to image
        
        void clearContents( bool c = false );                                   // Clears contents; if c switches type to blank
        
        void setAutoResize( bool a = true );                                    // Auto-resize based on contents, false by default
        bool getAutoResize();
        
        void setTogglable( bool t = true );                                     // Togglable button, false by default
        bool getTogglable();
        
        // void setCallback( std::string cb );
    };
}

/******************************************************************************//******************************************************************************/

#endif


