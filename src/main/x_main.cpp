/* 
 * x_main.cpp
 * 
 * jb_main() wrapper for the X Window System
 * 
 * Currently using Xlib because:
 *   - XCB documentation is poor(er)
 *   - XCB & OpenGL don't mix too well, as GLX is tightly integrated with Xlib
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_main.h"

#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_platform.h"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    Display* x_display;
}

/******************************************************************************//******************************************************************************/

Display* getXDisplay()                                                          // Declared in jb_platform.h
{
    return x_display;
}

int main( int argc, char* argv[] )
{
    int return_code = 0x00;
    
    if( !XInitThreads() && jade::getDevMode() )                                 // Using XInitThreads makes the TASK_SYSTEM mask unneeded on X-based platforms,
                                                                                // however other platforms still require single-threaded system code.
                                                                                // TODO: Potentially use TASK_ANY instead of TASK_SYSTEM wherever it is used
        ff::write( jb_out, "Warning: Could not initialize Xlib thread safety, attempting to start anyways\n" );
    
    x_display = XOpenDisplay( NULL );
    
    if( x_display == NULL )
        ff::write( jb_out, "Could not connect to X server\n" );
    else
    {
        jb_registerLaunchArgs();
        
        if( jb_parseLaunchArgs( argc, argv ) )
            return_code = jb_main();
        
        // DEBUG:
        ff::write( jb_out, "here...\n" );
        
        XCloseDisplay( x_display );
        
        // DEBUG:
        ff::write( jb_out, "and here...\n" );
    }
    
    // DEBUG:
    ff::write( jb_out, "asdf\n" );
    
    return return_code;
}


