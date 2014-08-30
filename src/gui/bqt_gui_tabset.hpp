#ifndef BQT_GUI_TABSET_HPP
#define BQT_GUI_TABSET_HPP

/* 
 * bqt_gui_tabset.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <map>

#include "bqt_gui_element.hpp"
#include "bqt_gui_group.hpp"
#include "bqt_gui_text_rsrc.hpp"

/******************************************************************************//******************************************************************************/

#define TABSET_BAR_HEIGHT       30
#define TABSET_TAB_HEIGHT       26

#define TABSET_SCROLL_FACTOR    40

#define TABSET_MIN_TAB_WIDTH    34
#define TABSET_MAX_TAB_WIDTH    142

#define TABSET_MAX_TITLE_WIDTH  TABSET_MAX_TAB_WIDTH - TABSET_MIN_TAB_WIDTH

namespace bqt
{
    class tabset : public gui_element
    {
    protected:
        struct tab_data
        {
            // tab* head;
            group* contents;
            text_rsrc* title;
            enum
            {
                CLOSE_SAFE,
                CLOSE_UNSAFE
            } state;
            enum
            {
                UP,
                OVER,
                DOWN
            } button_state;
            int position;
            int width;
        };
        std::vector< tab_data > tabs;
        int current_tab;
        int total_tab_width;
        
        int bar_scroll;
        
        bool capturing;
        bqt_platform_idevid_t captured_dev;
        float capture_start[ 3 ];
        
        int getTabIndex( group* g );
        void reorganizeTabs();
    public:
        tabset( window& parent,
               int x,
               int y,
               unsigned int w,
               unsigned int h );                                                // w, h of area below bar
        ~tabset();
        
        void addTab( group* g, std::string t );
        void removeTab( group* g );
        
        void setTabTitle( group* g, std::string t );
        void setTabSafe( group* g, bool safe );
        void makeTabCurrent( group* g );
        // void moveTabToLeft( group* g );
        // void moveTabToRight( group* g );
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


