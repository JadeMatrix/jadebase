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

#include "jb_platform.h"
#include "jb_window.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void registerWindow( window& w );
    void deregisterWindow( window& w);
    
    bool isRegisteredWindow( jb_platform_window_t& w );
    
    int getRegisteredWindowCount();
    
    window& getAnyWindow();                                                     // Necessary for sharing OpenGL contexts
    window& getWindow( jb_platform_window_t& w );                               // Throws an exception if no window matches the platform window
    
    void closeAllWindows();                                                     // Meant for cleanup; must be called from a task thread with TASK_SYSTEM
    
    void redrawAllWindows();                                                    // Usefule when we update a grpahical preference, for example
}

/******************************************************************************//******************************************************************************/

#endif


