/* 
 * cocoa_appdelegate.m
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#import "cocoa_appdelegate.h"

#import "jb_main.h"
#import "../utility/jb_settings.hpp"
#import "../windowsys/cocoa_events.h"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

@implementation AppDelegate
 
- ( void )applicationDidFinishLaunching:( NSNotification* )aNotification
{
    NSEvent* ( ^handler_block )( NSEvent* ) = ^( NSEvent* event )
    {
        return jade::handleCocoaEvent( event );
    };
    [ NSEvent addLocalMonitorForEventsMatchingMask: NSAnyEventMask
                                           handler: handler_block ];
    
    jb_main();
}

- ( BOOL )applicationShouldTerminateAfterLastWindowClosed:( NSApplication* )theApplication
{
    bool quit_on_no_windows;
    if( !jade::getSetting( "jb_QuitOnNoWindows", quit_on_no_windows ) )
        return NO;
    else
        return quit_on_no_windows;
}

@end


