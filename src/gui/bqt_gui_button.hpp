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
        
        std::string str_contents;
        //??? img_contents;
        
        bool auto_resize;
        
        bool togglable;
        enum
        {
            UP_OFF,
            UP_ON,
            DOWN_OFF,
            DOWN_ON
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
        std::pair< unsigned int, unsigned int > getMinDimensions();
        
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


