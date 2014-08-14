#ifndef BQT_LAYOUT_CANVAS_HPP
#define BQT_LAYOUT_CANVAS_HPP

/* 
 * bqt_layout_canvas.hpp
 * 
 * GUI element for displaying one or more canvasses in one or more views
 * 
 * Each child pane has an associated float value 0.0 ... 1.0, which is its
 * width/height as a fraction of its' parent's.  Each pane also has a pair of
 * dividers which are included in its width.  So the viewport for rendering a
 * canvas is float * parent_dim - divider_width.
 * The root pane divides vertically (like pane | pane ), the direction alter-
 * nates down from there.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_layout.hpp"

#include <vector>

#include "bqt_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // TODO: Split this into canvas_element and pane_element, which can then be
    // combined at a higher level
    
    class canvas_element : public layout_element
    {
    protected:
        struct pane
        {
            enum
            {
                EMPTY,
                CANVAS,
                PANES
            } type;
            std::vector< std::pair< pane, float > > children;                   // Should be empty if contents non-NULL
            canvas* contents;
            
            mutex pane_mutex;
            
            void draw( unsigned int w,
                       unsigned int h,
                       bool vert = true );
        };
        
        pane root;
    public:
        canvas_element( int x,
                        int y,
                        unsigned int w,
                        unsigned int h );
        
        layout_element* acceptEvent( window_event& e );
        
        void draw();
    };
    
    
}

/******************************************************************************//******************************************************************************/

#endif


