/* 
 * bqt_events.cpp
 * 
 * Clicks & strokes are both converted to stroke events & passed to windows,
 * as it is entirely up to the window to check the event position(s) and thus to
 * decide whether it is a drag or a stroke.
 * 
 * Also implements setQuitFlag() and getQuitFlag() from bqt_platform.h
 * 
 * Useful reference:
 *     http://tronche.com/gui/x/xlib/events/structures.html#XEvent
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_events.hpp"

#include <map>
#include <vector>
#include <cmath>

#include "bqt_taskexec.hpp"
#include "bqt_windowmanagement.hpp"
#include "bqt_window.hpp"
#include "bqt_windowevent.hpp"
#include "bqt_platform.h"
#include "bqt_exception.hpp"
#include "bqt_launchargs.hpp"
#include "threading/bqt_mutex.hpp"
#include "bqt_keycode.hpp"
#include "bqt_preferences.hpp"
#include "gui/bqt_named_resources.hpp"
#include "bqt_log.hpp"

#include "x_inputdevices.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    std::map< Window, bqt::window::manipulate* > window_manipulates;
    
    // X QUIT HANDLING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #ifdef PLATFORM_XWS_GNUPOSIX
    
    bqt::mutex quit_mutex;
    bool quit_flag = false;
    
    #endif
    
    // EVENT HANDLERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    void handleKeyEvent( XEvent& x_event )
    {
        bqt_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        bqt::window* target_window = &bqt::getWindow( platform_window );
        
        if( target_window == NULL )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Got key event for unknown window, using active window\n" );
            target_window = bqt::getActiveWindow();
        }
        
        if( target_window == NULL )
        {
            if( bqt::getDevMode() )
                ff::write( bqt_out, "Got key event with no active window, ignoring\n" );
        }
        else
        {
            bqt::window_event w_event;
            w_event.type = bqt::KEYCOMMAND;
            
            switch( x_event.xkey.type )
            {
                case KeyPress:
                    w_event.key.up = false;
                    break;
                case KeyRelease:
                    w_event.key.up = true;
                    break;
                default:
                    throw bqt::exception( "handleKeyEvent(): Key event neither up nor down" );
            }
            
            w_event.key.key = bqt::convertPlatformKeycode( XLookupKeysym( &x_event.xkey, 0 ) );
            
            if( w_event.key.key != bqt::KEY_INVALID )                           // Simply ignore invalid keys
            {
                w_event.key.shift = ( bool )( x_event.xkey.state & ShiftMask );
                w_event.key.ctrl  = ( bool )( x_event.xkey.state & ControlMask );
                w_event.key.alt   = ( bool )( x_event.xkey.state & Mod1Mask );
                w_event.key.super = ( bool )( x_event.xkey.state & Mod4Mask );      // Run xmodmap to find these on a given system
                
                #ifdef PLATFORM_MACOSX
                w_event.key.cmd = w_event.key.super;
                #else
                w_event.key.cmd = w_event.key.ctrl;
                #endif
                
                target_window -> acceptEvent( w_event );
            }
        }
    }
    
    void handleTextEvent( XEvent& x_event )
    {
        // http://www.x.org/releases/X11R7.6/doc/libX11/specs/XIM/xim.html
    }
    
    void handleWindowEvent( XEvent& x_event )
    {
        using namespace bqt;
        
        bqt_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        
        try
        {
            bqt::window& bqt_window( getWindow( platform_window ) );            // Try block is for this statement
            
            if( isRegisteredWindow( platform_window ) )
            {
                if( !window_manipulates.count( platform_window.x_window ) )
                    window_manipulates[ platform_window.x_window ] = new window::manipulate( &bqt_window );
            }
            else
            {
                if( getDevMode() )
                    ff::write( bqt_out, "X window event received for unregistered window, ignoring\n" );
                
                return;
            }
            
            window::manipulate* current_manip = window_manipulates[ platform_window.x_window ];
            
            switch( x_event.type )
            {
            // case :  // Shown
            //     break;
            case Expose:
                if( x_event.xexpose.count != 0 )
                    current_manip -> redraw();
                current_manip -> makeActive();
                break;
            case ConfigureRequest:
                // ff::write( bqt_out, "ConfigureRequest\n" );
                // x_event.xconfigurerequest
                break;
            case ConfigureNotify:
                {
                    // if( x_event.xconfigure.value_mask & CWX || x_event.xconfigure.value_mask & CWY )
                    if( x_event.xconfigure.x !=bqt_window.getPosition().first
                        || x_event.xconfigure.y !=bqt_window.getPosition().second )
                    {
                        current_manip -> setPosition( x_event.xconfigure.x, x_event.xconfigure.y );
                    }
                    
                    // if( x_event.xconfigure.value_mask & CWWidth || x_event.xconfigure.value_mask & CWHeight )
                    if( x_event.xconfigure.width != bqt_window.getDimensions().first
                        || x_event.xconfigure.height != bqt_window.getDimensions().second )
                    {
                        if( x_event.xconfigure.width < 1
                            || x_event.xconfigure.height < 1 )                      // Trust no one
                            throw exception( "handleWindowEvent(): Width or height not within limits" );
                        
                        // ff::write( bqt_out, "Setting dimensions\n" );
                        current_manip -> setDimensions( x_event.xconfigure.width,
                                                        x_event.xconfigure.height );
                    }
                }
                break;
            case MapNotify:
            case MapRequest:
            case VisibilityNotify:
            case FocusIn:
                current_manip -> restore();
                current_manip -> makeActive();
                break;
            case FocusOut:
                current_manip -> makeInactive();
                break;
            case ClientMessage:
                // http://tronche.com/gui/x/icccm/sec-4.html#s-4.2.8.1
                if( x_event.xclient.data.l[ 0 ] == XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False ) )
                    current_manip -> close();
                break;
            // case :  // Minimize
            //     current_manip -> minimize();
            //     break;
            // case :  // Maximize
            //     current_manip -> maximize();
            //     break;
            // case :  // Mouse Focus
            //     break;
            // case :  // Keyboard Focus
            //     break;
            default:
                if( getDevMode() )
                    ff::write( bqt_out, "Received unrecognized X window event, ignoring\n" );
                break;
            }
        }
        catch( exception& e )
        {
            if( x_event.type != FocusOut )                                      // Window close generates a FocusOut event, but window is already gone
                throw e;
            else
                if( getDevMode() )
                    ff::write( bqt_out, "Got FocusOut X event for invalid window, assumed closed\n" );
        }
    }
    
    #endif
}

/******************************************************************************//******************************************************************************/

#if defined PLATFORM_XWS_GNUPOSIX

void setQuitFlag()
{
    bqt::scoped_lock< bqt::mutex > slock( quit_mutex );
    quit_flag = true;
}
int getQuitFlag()
{
    bqt::scoped_lock< bqt::mutex > slock( quit_mutex );
    return ( int )quit_flag;
}

namespace bqt
{
    bool HandleEvents_task::execute( task_mask* caller_mask )
    {
        if( getQuitFlag() )
        {
            if( getDevMode() )
                ff::write( bqt_out, "Quitting...\n" );
            
            #warning Quitting does not check open documents
            if( false /* !closeAllDocuments() */ )
            {
                scoped_lock< mutex > slock( quit_mutex );
                quit_flag = false;
                
                ff::write( bqt_out, "Quit interrupted\n" );
            }
            else
            {
                closeInputDevices();
                closeAllWindows();
                
                deInitNamedResources();
                
                submitTask( new StopTaskSystem_task() );
            }
        }
        else
        {
            XEvent x_event;
            Display* x_display = getXDisplay();
            
            refreshInputDevices();
            
            XEvent last_x_dmevent;                                              // For storing DeviceMotion events until we can fill out their window field
            bool dmevent_waiting = false;
            
            for( int queue_size = XEventsQueued( x_display, QueuedAfterFlush ); // AKA XPending( x_display )
                 queue_size > 0;
                 --queue_size )                                                 // Yay we can guarantee termination
            {
                XNextEvent( x_display, &x_event );
                
                
                
                switch( x_event.type )
                {
                case KeyRelease:
                    if( XEventsQueued( x_display, QueuedAfterReading ) )        // Skip key repeats
                    {
                        XEvent x_nextevent;
                        XPeekEvent( x_display, &x_nextevent );
                        
                        if( x_nextevent.type == KeyPress
                            && x_nextevent.xkey.time == x_event.xkey.time
                            && x_nextevent.xkey.keycode == x_event.xkey.keycode )   // Key repeat
                        {
                            XNextEvent( x_display, &x_event );                  // Get the repeated key press
                            XNextEvent( x_display, &x_event );                  // Get the event after the repeat
                        }
                    }                                                           // Fall through
                case KeyPress:
                    handleKeyEvent( x_event );
                    break;
                case Expose:
                case ConfigureRequest:
                case ConfigureNotify:
                case MapNotify:
                case MapRequest:
                case ClientMessage:
                case VisibilityNotify:
                case FocusIn:
                case FocusOut:
                    handleWindowEvent( x_event );
                    break;
                case DestroyNotify:
                case CreateNotify:
                case UnmapNotify:
                case ReparentNotify:
                case GravityNotify:
                case EnterNotify:
                case LeaveNotify:
                case GraphicsExpose:
                case NoExpose:
                case CirculateNotify:
                case CirculateRequest:
                case SelectionClear:
                case SelectionRequest:
                case SelectionNotify:
                case ColormapNotify:
                case PropertyNotify:
                    break;                                                      // Ignore, for now
                case MotionNotify:
                    if( dmevent_waiting )
                    {
                        XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&last_x_dmevent ) );
                        
                        if( x_dmevent.time == x_event.xmotion.time )
                        {
                            x_dmevent.window = x_event.xmotion.window;          // Copy over window
                            
                            handleStrokeEvent( last_x_dmevent );
                            
                            dmevent_waiting = false;
                        }
                    }
                    break;
                case ButtonPress:
                case ButtonRelease:
                    if( dmevent_waiting )
                    {
                        XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&last_x_dmevent ) );
                        
                        if( x_dmevent.time == x_event.xbutton.time )
                        {
                            x_dmevent.window = x_event.xmotion.window;          // Copy over window
                            
                            handleStrokeEvent( last_x_dmevent );
                            
                            dmevent_waiting = false;
                        }
                    }
                    break;
                default:
                    last_x_dmevent = x_event;                                   // Save event so we can fill out the window field later
                    dmevent_waiting = true;
                    break;
                }
            }
            
            for( std::map< Window, bqt::window::manipulate* >::iterator iter = window_manipulates.begin();
                 iter != window_manipulates.end();
                 ++iter )
            {
                submitTask( iter -> second );
            }
            window_manipulates.clear();
        }
        
        return false;                                                           // Requeue instead of submitting a new copy
    }
}

#else

#error "Events not implemented on non-X platforms"

#endif


