#ifndef BQT_WINDOWMANAGEMENT_HPP
#define BQT_WINDOWMANAGEMENT_HPP

/* 
 * bqt_windowmanagement.hpp
 * 
 * Internat window management for events (usually which one is active)
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_platform.h"
#include "bqt_window.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void registerWindow( window& w );
    void deregisterWindow( window& w);
    
    bool isRegisteredWindow( bqt_platform_window_t& w );
    
    int getRegisteredWindowCount();
    
    void makeWindowActive(   bqt_platform_window_t& w );
    void makeWindowInactive( bqt_platform_window_t& w );
    window* getActiveWindow();                                                  // Returns NULL if there is no active window
    window& getWindow( bqt_platform_window_t& w );                              // Throws an exception if no window matches the platform window
    
    void closeAllWindows();                                                     // Meant for cleanup; must be called from a task thread with TASK_SYSTEM
    
    void redrawAllWindows();                                                    // Usefule when we update a grpahical preference, for example
}

/******************************************************************************//******************************************************************************/

#endif


