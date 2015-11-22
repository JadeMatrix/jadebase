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

// TODO: Figure out some way for this to call jade::getSetting( "jb_QuitOnNoWindows", ... )

// - ( BOOL )applicationShouldTerminateAfterLastWindowClosed:( NSApplication* )theApplication
// {
//     return NO;
// }

@end


