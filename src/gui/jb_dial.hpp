#ifndef JADEBASE_DIAL_HPP
#define JADEBASE_DIAL_HPP

/* 
 * jb_dial.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <memory>

#include "jb_element.hpp"
#include "../utility/jb_callback.hpp"

/******************************************************************************//******************************************************************************/

#define DIAL_LARGE_DIAMETER 42
#define DIAL_SMALL_DIAMETER 22

#define DIAL_MAX_VALUE      1.0f
#define DIAL_MIN_VALUE     -1.0f
#define DIAL_DEFAULT_VALUE  0.0f

#define DIAL_DRAG_FACTOR    80.0f

namespace jade
{
    class dial : public gui_element
    {
    protected:
        float value;
        bool small;
        enum
        {
            NONE,
            VERTICAL,
            CIRCULAR
        } capturing;
        jb_platform_idevid_t captured_dev;
        float capture_start[ 3 ];
        
        std::shared_ptr< callback > value_changed_callback;
    public:
        dial( window* parent,
              int x,
              int y,
              bool s = false,
              float v = DIAL_DEFAULT_VALUE );
        
        float getValue();
        void setValue( float v );
        
        void setValueChangedCallback( const std::shared_ptr< callback >& );
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


