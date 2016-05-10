#ifndef JADEBASE_COCOA_UTIL_H
#define JADEBASE_COCOA_UTIL_H

/* 
 * jb_cocoa_util.hpp
 * 
 * Utilities for interacting with Cocoa
 * 
 * The reason jade::cocoa::bridging_downcast() & jade::cocoa::bridging_upcast()
 * are a pair of functions rather than a wrapper class is because jb_platform.h,
 * a pure-C header file, must define jb_platform_window_t etc., which contain
 * some direct representation of the platform types.
 * 
 * A word of warning: all STLSoft headers (via FastFormat) must be included
 * before any OS X headers, as the OS X macro 'check' in AssertMacros.h
 * conflicts with the struct method 'check' in stlsoft.h.
 * 
 */

#ifndef __OBJC__
#error jb_cocoa_util.hpp is an Objective-C++ header file
#endif

/* INCLUDES *******************************************************************//******************************************************************************/

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

/******************************************************************************//******************************************************************************/

namespace jade
{
    namespace cocoa
    {
        template< typename NSType > void* bridging_downcast( NSType* o )        // Retain an NSObject of some type and cast to a void*
                                                                                // Does not strictly need to be a template function, but is as a counterpart to
                                                                                // jade::cocoa::bridging_upcast()
        {
            return ( void* )CFBridgingRetain( o );
        }
        
        template< typename NSType > NSType* bridging_upcast( void* v )          // Cast an NSObject of some type from a void* but do not release
        {
            NSType* o = ( NSType* )CFBridgingRelease( v );
            CFBridgingRetain( o );
            return o;
        }
    }
}

/******************************************************************************//******************************************************************************/

#endif


