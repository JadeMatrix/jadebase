/* 
 * cocoa_window.mm
 * 
 * Platform-specific code from jb_window.hpp on Cocoa-based platforms
 * 
 */

#ifndef PLATFORM_MACOSX
#error PLATFORM_MACOSX not defined
#endif

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

#import "../utility/jb_cocoa_util.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    dpi::pixels window::getDPI()
    {
        
    }
    
    jb_platform_window_t& window::getPlatformWindow()
    {
        
    }
    
    void window::init()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        
    }
    
    void window::makeContextCurrent()
    {
        
    }
    void window::swapBuffers()
    {
        
    }
    
    window::~window()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        
    }
    
    /* window::manipulate *****************************************************//******************************************************************************/
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        
    }
}


