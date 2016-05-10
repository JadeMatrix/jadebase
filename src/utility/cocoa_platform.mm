/* 
 * cocoa_platform.mm
 * 
 * Implementation of jb_platform.h for Cocoa
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_platform.h"

#import "jb_cocoa_util.hpp"

/******************************************************************************//******************************************************************************/

int jb_platform_window_t_compare( const jb_platform_window_t left,
                                  const jb_platform_window_t right )
{
    NSWindow* ns_left  = jade::cocoa::bridging_upcast< NSWindow >(  left.cf_retained_nswindow );
    NSWindow* ns_right = jade::cocoa::bridging_upcast< NSWindow >( right.cf_retained_nswindow );
    
    return ( int )( ns_left.windowNumber - ns_right.windowNumber );
}

int jb_platform_idevid_t_compare( const jb_platform_idevid_t left,
                                  const jb_platform_idevid_t right )
{
    if(    left.id_type  == NS_MOUSE
        && right.id_type == NS_MOUSE )
        return 0;
    
    if( left.ns_tablet_sysid == right.ns_tablet_sysid )
        return left.ns_pointer_sysid - right.ns_pointer_sysid;
    
    return left.ns_tablet_sysid - right.ns_tablet_sysid;
}


