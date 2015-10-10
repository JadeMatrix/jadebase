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
#include <memory>

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
    public:
        class tab
        {
            friend class tabset;
            
        public:
            tab( tabset*,                                                       // Parent tabset (can be NULL)
                 std::string,                                                   // Title
                 std::shared_ptr< group >& );                                   // Tab contents
            ~tab();
            
            void setTitle( std::string );
            std::string getTitle();
            
            void setSafe( bool );                                               // Set whether the tab's close button is in safe-to-close mode
            bool getSafe();                                                     // Get the above mode
            
            void setParentTabset( tabset* );
            
        protected:
            mutex tab_mutex;
            tabset* parent;
            std::shared_ptr< group > contents;
            text_rsrc* title;
            bool safe;
        };
        
        tabset( dpi::points,                                                    // x position
                dpi::points,                                                    // y position
                dpi::points,                                                    // width of area below bar
                dpi::points );                                                  // height of area below bar
        ~tabset();                                                              // Calls closed() on all tabs' contents
        
        void addTab(    std::shared_ptr< tab >& );
        void removeTab( std::shared_ptr< tab >& );                              // Does not call the content's closed()
        
        void makeTabCurrent( std::shared_ptr< tab >& );
        void moveTabLeft(    std::shared_ptr< tab >& );
        void moveTabRight(   std::shared_ptr< tab >& );
        
        void setParentElement( gui_element* );
        
        void setRealPosition(   dpi::points, dpi::points );                     // x, y
        void setRealDimensions( dpi::points, dpi::points );                     // w, h
        
        bool acceptEvent( window_event& );
        
        void draw( window* );
        
    protected:
        struct tab_state
        {
            std::shared_ptr< tab > data;
            
            enum
            {
                UP,
                OVER,
                DOWN
            } button_state;
            dpi::points position;
            dpi::points width;
            
            tab_state( std::shared_ptr< tab >& t ) : data( t ) {}               // Using std::shared_ptr<>& here is safe as it is copied on storage
        };
        std::vector< tab_state > tabs;
        int current_tab;
        dpi::points total_tab_width;
        
        dpi::points bar_scroll;
        
        bool capturing;
        jb_platform_idevid_t captured_dev;
        dpi::points capture_start[ 3 ];
        
        int getTabIndex( std::shared_ptr< tab > );
        void reorganizeTabs();
    };
}

/******************************************************************************//******************************************************************************/

#endif


