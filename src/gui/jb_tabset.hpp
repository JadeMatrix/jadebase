#ifndef JADEBASE_TABSET_HPP
#define JADEBASE_TABSET_HPP

/* 
 * jb_tabset.hpp
 * 
 * Tab contents are deleted on tab close if setting
 * "jb_DeleteTabContentsAfterClose" is true
 * 
 * Warning: tab contents can change size or be closed while hidden
 * 
 */

// TODO: Capturing for tab close buttons

/* INCLUDES *******************************************************************//******************************************************************************/

#include <map>

#include "jb_element.hpp"
#include "jb_group.hpp"
#include "jb_text_rsrc.hpp"
#include "../utility/jb_container.hpp"

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
    public:
        class tab
        {
            friend class tabset;
        protected:
            mutex tab_mutex;
            tabset* parent;
            container< group > contents;
            text_rsrc* title;
            bool safe;
        public:
            tab( tabset*,                                                       // Parent tabset (can be NULL)
                 std::string,                                                   // Title
                 container< group >& );                                         // Using container<>& here is safe as it is copied on storage
            ~tab();
            
            void setTitle( std::string );
            std::string getTitle();
            
            void setTabSafe( bool );                                            // Set whether the tab's close button is in safe-to-close mode
            bool getTabSafe();                                                  // Get the above mode
            
            void setParentTabset( tabset* );
        };
        
        tabset( window*,
                int,                                                            // x position
                int,                                                            // y position
                unsigned int,                                                   // width of area below bar
                unsigned int );                                                 // height of area below bar
        ~tabset();                                                              // Calls closed() on all tabs' contents
        
        void addTab( container< tab > );
        void removeTab( container< tab > );                                     // Does not call the content's closed()
        
        void makeTabCurrent( container< tab > );
        void moveTabToLeft( container< tab > );
        void moveTabToRight( container< tab > );
        
        void setParentWindow( window* );
        
        void setRealPosition( int, int );                                       // x, y
        void setRealDimensions( unsigned int, unsigned int );                   // w, h
        
        bool acceptEvent( window_event& );
        
        void draw();
    protected:
        struct tab_state
        {
            container< tab > data;
            
            enum
            {
                UP,
                OVER,
                DOWN
            } button_state;
            int position;
            int width;
            
            tab_state( container< tab >& t ) : data( t ) {}                     // Using container<>& here is safe as it is copied on storage
        };
        std::vector< tab_state > tabs;
        int current_tab;
        int total_tab_width;
        
        int bar_scroll;
        
        bool capturing;
        jb_platform_idevid_t captured_dev;
        float capture_start[ 3 ];
        
        int getTabIndex( container< tab > );
        void reorganizeTabs();
    };
}

/******************************************************************************//******************************************************************************/

#endif


