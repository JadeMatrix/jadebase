/* 
 * AppDelegate.m
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#import "cocoa_appdelegate.h"

#import "jb_main.h"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

@implementation AppDelegate
 
- ( void )applicationDidFinishLaunching:( NSNotification* )aNotification
{
    jb_main();
}

// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed:( NSApplication* )theApplication
// {
//     return NO;
// }

@end


