#ifndef JADEBASE_WINDOWMANAGEMENT_HPP
#define JADEBASE_WINDOWMANAGEMENT_HPP

/* 
 * jb_windowmanagement.hpp
 * 
 * Internal window management for inter-window communication & window memory
 * management
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"
#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void registerWindow( window& );
    void deregisterWindow( window& );
    
    bool isRegisteredWindow( jb_platform_window_t& );
    
    int getRegisteredWindowCount();
    
    window& getAnyWindow();                                                     // Necessary for sharing OpenGL contexts
    window& getWindow( jb_platform_window_t& );                                 // Throws an exception if no window matches the platform window
    
    void closeAllWindows();                                                     // Meant for cleanup; must be called from a task thread with TASK_SYSTEM
    
    void redrawAllWindows();                                                    // Useful when we update a global graphical setting
}

/******************************************************************************//******************************************************************************/

#endif


