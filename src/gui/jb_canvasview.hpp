#ifndef JADEBASE_CANVASVIEW_HPP
#define JADEBASE_CANVASVIEW_HPP

/* 
 * jb_canvasview.hpp
 * 
 * GUI element for displaying & passing drawing events to canvasses
 * 
 */

// TODO: Abstract/remove this in light of move from BQTDraw to jadebase

/* INCLUDES *******************************************************************//******************************************************************************/

#include <queue>
#include <set>

#include "jb_scrollable.hpp"
#include "../windowsys/jb_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class canvas_view : public scrollable
    {
    protected:
        // unsigned char* pixel_data;
        bool init;
        unsigned int data_dims[ 2 ];
        GLuint canvas_tex;
        GLuint canvas_fb;
        
        std::queue< stroke_waypoint > pending_waypoints;
        std::set< jb_platform_idevid_t > captured_devs;
        
        int scroll_limits[ 2 ];                                                 // X min, X max, Y min, Y max
        int scroll_offset[ 2 ];
    public:
        canvas_view( window& parent,
                     int x,
                     int y,
                     unsigned int w,
                     unsigned int h,
                     unsigned int c_w,
                     unsigned int c_h );
        ~canvas_view();
        
        // void closeContents();
        
        // GUI_ELEMENT /////////////////////////////////////////////////////////
        
        bool acceptEvent( window_event& e );
        
        void draw();
        
        // SCROLLABLE //////////////////////////////////////////////////////////
        
        void setRealDimensions( unsigned int w, unsigned int h );
        
        void scrollPixels(    int x,   int y );
        void scrollPercent( float x, float y );
        
        void setScrollPixels(    int x,   int y );
        void setScrollPercent( float x, float y );
        
        std::pair<   int,   int > getScrollPixels();
        std::pair< float, float > getScrollPercent();
        
        bool hasScrollLimit();
        std::pair<   int,   int > getScrollLimitPixels();
        std::pair< float, float > getScrollLimitPercent();
    };
}

/******************************************************************************//******************************************************************************/

#endif


