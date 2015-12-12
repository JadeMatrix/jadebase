/* 
 * cocoa_window.mm
 * 
 * Platform-specific code from jb_window.hpp on Cocoa-based platforms
 * 
 */

#ifndef PLATFORM_MACOSX
#error PLATFORM_MACOSX not defined
#endif

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_window.hpp"

#include <cmath>

#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"

#import "../utility/jb_cocoa_util.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

@interface JBWindowView : NSOpenGLView
    
@end

@implementation JBWindowView
    
    // TODO: Event handlers
    
@end

namespace
{
    NSOpenGLPixelFormatAttribute nsgl_pf_attrs[] = { NSOpenGLPFAColorSize, 24,
                                                     NSOpenGLPFAAlphaSize, 8,
                                                     NSOpenGLPFADoubleBuffer,
                                                     NSOpenGLPFAAccelerated,
                                                     0 };
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    /* window *****************************************************************//******************************************************************************/
    
    dpi::pixels window::getDPI()
    {
        return getScaleFactor() * STANDARD_DPI;
    }
    dpi::percent window::getScaleFactor()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        float scale_override = getGUIScaleOverride();
        
        if( isnan( scale_override) )
        {
            NSWindow* ns_window = cocoa::bridging_upcast< NSWindow >( platform_window.cf_retained_nswindow );
            
            NSPoint p = { 1.0f, 1.0f };
            
            return [ [ ns_window contentView ] convertPointToBacking: p ].y;
        }
        else
            return scale_override;
    }
    
    jb_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        if( platform_window.cf_retained_nswindow == nullptr )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    void window::platformWindowConstructor()
    {
        platform_window.cf_retained_nswindow = nullptr;
    }
    void window::init()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        // Set up window view & OpenGL /////////////////////////////////////////
        
        JBWindowView* view = [ [ JBWindowView alloc ] init ];
        
        NSOpenGLPixelFormat* format = [ [ NSOpenGLPixelFormat alloc ] initWithAttributes: nsgl_pf_attrs ];
        
        [ view setPixelFormat: format ];
        
        GLint swapInt = 1;
        [ [ view openGLContext ] setValues: &swapInt
                                 forParameter: NSOpenGLCPSwapInterval ];        // NSOpenGLCPSwapInterval = 1 turns on vsync
        
        // Set up window ///////////////////////////////////////////////////////
        
        NSRect frame = NSMakeRect( position[ 0 ],
                                   position[ 1 ],
                                   dimensions[ 0 ],
                                   dimensions[ 1 ] );
        
        NSWindow* ns_window = [ [ NSWindow alloc ] initWithContentRect: frame
                                                   styleMask: ( NSClosableWindowMask
                                                                | NSTitledWindowMask
                                                                | NSMiniaturizableWindowMask
                                                                | NSResizableWindowMask )
                                                   backing: NSBackingStoreBuffered
                                                   defer: NO ];
        
        [ ns_window makeKeyAndOrderFront: NSApp ];
        [ ns_window setContentView: view ];
        
        // Store window in platform window /////////////////////////////////////
        
        platform_window.cf_retained_nswindow = cocoa::bridging_downcast< NSWindow >( ns_window );
    }
    
    void window::makeContextCurrent()
    {
        NSWindow* ns_window = cocoa::bridging_upcast< NSWindow >( platform_window.cf_retained_nswindow );
        
        [ [ [ ns_window contentView ] openGLContext ] makeCurrentContext ];
    }
    void window::swapBuffers()
    {
        NSWindow* ns_window = cocoa::bridging_upcast< NSWindow >( platform_window.cf_retained_nswindow );
        
        [ [ [ ns_window contentView ] openGLContext ] flushBuffer ];
        
        // GLenum gl_error;
        // if( ( gl_error = glGetError() ) != GL_NO_ERROR )
        // {
        //     jade::exception e;
        //     ff::write( *e,
        //                "window::swapBuffers(): Error flushing buffer: ",
        //                getGLError( gl_error ) );
        //     throw e;
        // }
    }
    
    window::~window()
    {
        scoped_lock< mutex > slock( window_mutex );
        
        CFBridgingRelease( platform_window.cf_retained_nswindow );              // Dereference NSWindow
    }
    
    /* window::manipulate *****************************************************//******************************************************************************/
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        // IMPLEMENT:
    }
}


