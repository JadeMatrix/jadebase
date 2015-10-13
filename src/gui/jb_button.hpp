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
    public:
        button( dpi::points,                                                    // Initial x position in points
                dpi::points,                                                    // Initial y position in points
                dpi::points w = BUTTON_MIN_WIDTH,                               // Initial width in points
                dpi::points h = BUTTON_MIN_HEIGHT );                            // Initial height in points
        
        void setToggleOnCallback(  const std::shared_ptr< callback >& );        // Setting these to empty shared_ptrs disables them
        void setToggleOffCallback( const std::shared_ptr< callback >& );
        
        void setRealDimensions( dpi::points, dpi::points );                     // Width, height
        
        void setContents( const std::shared_ptr< gui_resource >&,               // Contents resource pointer
                          resource_align,                                       // Content alignment
                          bool r = false );                                     // Resize button to match contents
                                                                                // TODO: x/xx/xxxx resize options
        // TODO: void setContentsPadding( dpi::points );
        //       void setContentsPadding( dpi::points, dpi::points );
        //       void setContentsPadding( dpi::points, dpi::points, dpi::points, dpi::points );
        
        bool acceptEvent( window_event& );
        
        void draw( window* );
        
        void clearDeviceAssociations();
        
    protected:
        button_state state;
        jb_platform_idevid_t captured_dev;
        
        std::shared_ptr< callback > toggle_on_callback;
        std::shared_ptr< callback > toggle_off_callback;
        
        std::shared_ptr< gui_resource > contents;
        resource_align contents_align;
        
        void setState( button_state );                                          // Internal utility function, not thread-safe
    };
}

/******************************************************************************//******************************************************************************/

#endif


