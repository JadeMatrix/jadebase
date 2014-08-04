/* 
 * x_main.cpp
 * 
 * bqt_main() wrapper for the X Window System
 * 
 * Currently using Xlib because:
 *   - XCB documentation is poor(er)
 *   - XCB & OpenGL don't mix too well, as GLX is tightly integrated with Xlib
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_main.h"

#include "bqt_platform.h"
#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    Display* x_display;
}

/******************************************************************************//******************************************************************************/

Display* getXDisplay()                                                          // Declared in bqt_platform.h
{
    return x_display;
}

int main( int argc, char* argv[] )
{
    int return_code = 0x00;
    
    x_display = XOpenDisplay( NULL );
    
    if( x_display == NULL )
        ff::write( bqt_out, "Could not connect to X server\n" );
    else
    {
        if( bqt_parseLaunchArgs( argc, argv ) )
            return_code = bqt_main();
        
        XCloseDisplay( x_display );
    }
    
    return return_code;
}


