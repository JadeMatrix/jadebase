/* 
 * bqt_main.m
 * 
 * Implements main() wrapping on OS X with Cocoa; see AppDelegate.h &
 * AppDelegate.m for more Cocoa wrapping.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#import <Cocoa/Cocoa.h>

#import "bqt_main.h"
#import "bqt_appdelegate_cocoa.h"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    if( bqt_parseLaunchArgs( argc, argv ) )
    {
        NSAutoreleasePool*    pool = [ [ NSAutoreleasePool alloc ] init ];
        NSApplication* application = [ NSApplication sharedApplication ];
        
        AppDelegate * applicationDelegate = [ [ [ AppDelegate alloc ] init ] autorelease ];
        [ application setDelegate:applicationDelegate ];
        [ application run ];
        
        [ pool drain ];
        
        return 0;
    }
    else
        return -1;
}


