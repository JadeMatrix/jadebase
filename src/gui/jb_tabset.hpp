#ifndef JADEBASE_TABSET_HPP
#define JADEBASE_TABSET_HPP

/* 
 * jb_tabset.hpp
 * 
 * Tab contents are deleted on tab close if setting
 * "jb_DeleteTabContentsAfterClose" is true
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <map>

#include "jb_element.hpp"
#include "jb_group.hpp"
#include "jb_text_rsrc.hpp"

/******************************************************************************//******************************************************************************/

#define TABSET_BAR_HEIGHT       30
#define TABSET_TAB_HEIGHT       26

#define TABSET_SCROLL_FACTOR    40

#define TABSET_MIN_TAB_WIDTH    34
#define TABSET_MAX_TAB_WIDTH    142

#define TABSET_MAX_TITLE_WIDTH  TABSET_MAX_TAB_WIDTH - TABSET_MIN_TAB_WIDTH

namespace jade
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
        jb_platform_idevid_t captured_dev;
        float capture_start[ 3 ];
        
        int getTabIndex( group* g );
        void reorganizeTabs();
    public:
        tabset( window& parent,
               int x,
               int y,
               unsigned int w,
               unsigned int h );                                                // w, h of area below bar
        ~tabset();                                                              // "Closes" all tabs if setting "jb_ChainGUICleanup" is true
                                                                                // ("jb_DeleteTabContentsAfterClose" still applies)
        
        void addTab( group* g, std::string t );                                 // tabset does NOT take control of the group pointer
        void removeTab( group* g );                                             // Does not delete or closed() the group
        
        void setTabTitle( group* g, std::string t );
        void setTabSafe( group* g, bool safe );
        void makeTabCurrent( group* g );
        void moveTabToLeft( group* g );
        void moveTabToRight( group* g );
        
        void setRealPosition( int x, int y );
        void setRealDimensions( unsigned int w, unsigned int h );
        
        bool acceptEvent( window_event& e );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


