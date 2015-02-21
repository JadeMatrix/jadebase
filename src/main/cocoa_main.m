/* 
 * jb_main.m
 * 
 * Implements main() wrapping on OS X with Cocoa; see cocoa_appdelegate.h &
 * cocoa_appdelegate.m for more Cocoa wrapping.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#import <Cocoa/Cocoa.h>

#import "cocoa_appdelegate.h"
#import "jb_main.h"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    jb_registerLaunchArgs();
    
    if( jb_parseLaunchArgs( argc, argv ) )
    {
        NSAutoreleasePool*    pool = [ [ NSAutoreleasePool alloc ] init ];
        NSApplication* application = [ NSApplication sharedApplication ];
        
        AppDelegate * applicationDelegate = [ [ [ AppDelegate alloc ] init ] autorelease ];
        [ application setDelegate:applicationDelegate ];
        [ application run ];
        
        [ pool drain ];
    }
    
    return 0;
}


