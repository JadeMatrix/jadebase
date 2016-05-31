/* 
 * cocoa_events.mm
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../utility/jb_log.hpp"                                                // Included first due to the 'check' macro

#include "../utility/jb_platform.h"                                             // Has to be included first because of glew.h/gltypes.h

#include "cocoa_events.h"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

// namespace
// {
    
// }

/******************************************************************************//******************************************************************************/

void jb_setQuitFlag()
{
    
}
int jb_getQuitFlag()
{
    return 0;
}

namespace jade
{
    NSEvent* handleCocoaEvent( NSEvent* ns_event )
    {
        switch( ns_event.type )
        {
        case NSLeftMouseDown:
            ff::write( jb_out, "NSLeftMouseDown" );
            break;
        case NSLeftMouseUp:
            ff::write( jb_out, "NSLeftMouseUp" );
            break;
        case NSRightMouseDown:
            ff::write( jb_out, "NSRightMouseDown" );
            break;
        case NSRightMouseUp:
            ff::write( jb_out, "NSRightMouseUp" );
            break;
        case NSMouseMoved:
            ff::write( jb_out, "NSMouseMoved" );
            break;
        case NSLeftMouseDragged:
            ff::write( jb_out, "NSLeftMouseDragged" );
            break;
        case NSRightMouseDragged:
            ff::write( jb_out, "NSRightMouseDragged" );
            break;
        case NSMouseEntered:
            ff::write( jb_out, "NSMouseEntered" );
            break;
        case NSMouseExited:
            ff::write( jb_out, "NSMouseExited" );
            break;
        case NSKeyDown:
            ff::write( jb_out, "NSKeyDown" );
            break;
        case NSKeyUp:
            ff::write( jb_out, "NSKeyUp" );
            break;
        case NSFlagsChanged:
            ff::write( jb_out, "NSFlagsChanged" );
            break;
        case NSAppKitDefined:
            ff::write( jb_out, "NSAppKitDefined" );
            break;
        case NSSystemDefined:
            ff::write( jb_out, "NSSystemDefined" );
            break;
        case NSApplicationDefined:
            ff::write( jb_out, "NSApplicationDefined" );
            break;
        case NSPeriodic:
            ff::write( jb_out, "NSPeriodic" );
            break;
        case NSCursorUpdate:
            ff::write( jb_out, "NSCursorUpdate" );
            break;
        case NSScrollWheel:
            ff::write( jb_out, "NSScrollWheel" );
            break;
        case NSTabletPoint:
            ff::write( jb_out, "NSTabletPoint" );
            break;
        case NSTabletProximity:
            ff::write( jb_out, "NSTabletProximity" );
            break;
        case NSOtherMouseDown:
            ff::write( jb_out, "NSOtherMouseDown" );
            break;
        case NSOtherMouseUp:
            ff::write( jb_out, "NSOtherMouseUp" );
            break;
        case NSOtherMouseDragged:
            ff::write( jb_out, "NSOtherMouseDragged" );
            break;
        case NSEventTypeGesture:
            ff::write( jb_out, "NSEventTypeGesture" );
            break;
        case NSEventTypeMagnify:
            ff::write( jb_out, "NSEventTypeMagnify" );
            break;
        case NSEventTypeSwipe:
            ff::write( jb_out, "NSEventTypeSwipe" );
            break;
        case NSEventTypeRotate:
            ff::write( jb_out, "NSEventTypeRotate" );
            break;
        case NSEventTypeBeginGesture:
            ff::write( jb_out, "NSEventTypeBeginGesture" );
            break;
        case NSEventTypeEndGesture:
            ff::write( jb_out, "NSEventTypeEndGesture" );
            break;
        case NSEventTypeSmartMagnify:
            ff::write( jb_out, "NSEventTypeSmartMagnify" );
            break;
        case NSEventTypeQuickLook:
            ff::write( jb_out, "NSEventTypeQuickLook" );
            break;
        // OS X 10.10+
        // case NSEventTypePressure:
        //     break;
        }
        
        return ns_event;
    }
}


