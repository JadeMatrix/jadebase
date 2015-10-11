#ifndef JADEBASE_WINDOWVIEW_HPP
#define JADEBASE_WINDOWVIEW_HPP

/* 
 * jb_windowview.hpp
 * 
 * GUI element class for the top-level element in a window
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_group.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class windowview : public group
    {
    public:
        windowview( window* );                                                  // Parent window (permanently associated)
        
        void setParentElement( gui_element* );                                  // No-op, but logs the fact it was called
        
        void requestRedraw();
        
    protected:
        window* parent_window;
        
        void   associateDevice( jb_platform_idevid_t,
                                std::list< gui_element* >& );
        void   associateDevice( jb_platform_idevid_t );
        void deassociateDevice( jb_platform_idevid_t );
    };
}

/******************************************************************************//******************************************************************************/

#endif


