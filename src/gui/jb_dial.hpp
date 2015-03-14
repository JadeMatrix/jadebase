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

// TODO: Make these settings
#define DIAL_MAX_VALUE      1.0f
#define DIAL_MIN_VALUE     -1.0f
#define DIAL_DEFAULT_VALUE  0.0f

namespace jade
{
    class dial : public gui_element
    {
    public:
        dial( window* parent,
              int,                                                              // Initial X position in pixels
              int,                                                              // Initial Y position in pixels
              bool small = false,                                               // If true, initializes as a small dial
              float v = DIAL_DEFAULT_VALUE );                                   // Initial dial value
        
        void setValue( float );                                                 // Clipped between DIAL_MAX_VALUE and DIAL_MIN_VALUE
        float getValue();
        
        void setSmall( bool );                                                  // Set whether the dial is small or large
        bool getSmall();
        
        void setValueChangeCallback( const std::shared_ptr< callback >& );      // Setting this to an empty shared_ptr disables it
        
        bool acceptEvent( window_event& );
        
        void draw();
        
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
        
        std::shared_ptr< callback > value_change_callback;
    };
}

/******************************************************************************//******************************************************************************/

#endif


