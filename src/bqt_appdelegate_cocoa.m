/* 
 * AppDelegate.m
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#import "bqt_appdelegate_cocoa.h"

#import "bqt_main.h"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

@implementation AppDelegate
 
- ( void )applicationDidFinishLaunching:( NSNotification* )aNotification
{
    bqt_main();
}

// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed:( NSApplication* )theApplication
// {
//     return NO;
// }

@end


