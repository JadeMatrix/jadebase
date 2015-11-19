#ifndef JADEBASE_QUITTING_HPP
#define JADEBASE_QUITTING_HPP

/* 
 * jb_quitting.hpp
 * 
 * Utilities for requesting & handling application quitting & cleanup
 * 
 * Implemented in jb_events.cpp
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_callback.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void requestQuit();
    void cancelQuit();                                                          // Calling this does nothing if isCleaningUp() is true
    bool isQuitting();
    bool isCleaningUp();
    
    void setQuitRequestCallback( const std::shared_ptr< callback >& );
    void setQuitCleanupCallback( const std::shared_ptr< callback >& );
}

/******************************************************************************//******************************************************************************/

#endif


