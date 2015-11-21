/* 
 * jb_events.cpp
 * 
 * Clicks & strokes are both converted to stroke events & passed to windows,
 * as it is entirely up to the window to check the event position(s) and thus
 * decide the meaning of the input.
 * 
 * Also implements setQuitFlag() and getQuitFlag() from jb_platform.h
 * 
 * Useful reference:
 *     http://tronche.com/gui/x/xlib/events/structures.html#XEvent
 */

// FIXME: Prioritize looking for events that would activate windows so we can click on a window for focus AND have it get that click event

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_events.hpp"
#include "../utility/jb_quitting.hpp"

#include <cmath>
#include <map>
#include <vector>

#include "jb_keycode.hpp"
#include "jb_window.hpp"
#include "jb_windowevent.hpp"
#include "jb_windowmanagement.hpp"
#include "../gui/jb_named_resources.hpp"
#include "../tasking/jb_taskexec.hpp"
#include "../threading/jb_mutex.hpp"
#include "../threading/jb_thread.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_platform.h"

#ifdef PLATFORM_XWS_GNUPOSIX
#include "x_inputdevices.hpp"
#endif

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Accumulators for reinterpreting various events
    // We don't need any thread safety here as events have to be single-threaded
    
    #ifdef PLATFORM_XWS_GNUPOSIX
    
    std::map< Window, jade::window::manipulate* > window_manipulates;
    
    #endif
    
    // GENERAL EVENT GLOBALS  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    jade::thread event_loop;
    jade::mutex event_loop_kill_mutex;
    
    #if defined PLATFORM_XWS_GNUPOSIX
    Atom wakeup_atom;
    #endif
    
    // QUIT HANDLING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    jade::mutex quit_mutex;
    bool quit_flag = false;
    bool quit_callback_pending = false;
    bool cleanup_flag = false;
    std::shared_ptr< jade::callback > quit_callback;
    std::shared_ptr< jade::callback > cleanup_callback;
    
    class QuitRequestCallback_task : public jade::task
    {
    private:
        #ifdef PLATFORM_XWS_GNUPOSIX
        
        void sendWakeupEvent()
        {
            Display* x_display = getXDisplay();
            XEvent x_event;
            
            x_event.type = ClientMessage;
            x_event.xclient.message_type = wakeup_atom;
            x_event.xclient.format = 8;
            
            // Window x_window = jade::getAnyWindow().getPlatformWindow().x_window;
            
            ff::write( jb_out,
                       ">>>\tXSendEvent(\t0x",
                       ff::to_X( ( unsigned long )x_display ),
                       ",\n\t\t\t0x",
                       // ff::to_X( ( unsigned long )DefaultRootWindow( x_display ) ),
                       ff::to_X( ( unsigned long )PointerWindow ),
                       // ff::to_X( ( unsigned long )x_window ),
                       ",\n\t\t\t0x00,\n\t\t\t0x00,\n\t\t\t0x",
                       ff::to_X( ( unsigned long )&x_event ),
                       " );\n" );
            ff::write( jb_out,
                       ">>> wakeup_atom = 0x",
                       ff::to_X( wakeup_atom ),
                       "\n" );
            
            if( !XSendEvent( x_display,
                             PointerWindow,
                             // DefaultRootWindow( x_display ),
                             // x_window,
                             // 0x00,
                             0x01,
                             0x00,
                             &x_event ) )
            {
                // jade::exception e;
                
                // switch( error )
                // {
                // case BadValue:
                //     // ff::write( *e,
                //     //            "QuitRequestCallback_task::sendWakeupEvent(): Got a BadValue" );
                //     throw jade::exception( "QuitRequestCallback_task::sendWakeupEvent(): Got a BadValue\n" );
                //     break;
                // case BadWindow:
                //     // ff::write( *e,
                //     //            "QuitRequestCallback_task::sendWakeupEvent(): Got a BadWindow" );
                //     throw jade::exception( "QuitRequestCallback_task::sendWakeupEvent(): Got a BadWindow\n" );
                //     break;
                // default:
                //     // ff::write( *e,
                //     //            "QuitRequestCallback_task::sendWakeupEvent(): Got an unknown error" );
                //     ff::write( jb_out,
                //                "QuitRequestCallback_task::sendWakeupEvent(): Got an unknown Status ",
                //                ff::to_X( error ),
                //                "\n" );
                //     break;
                // }
                
                // throw e;
                
                throw jade::exception( "QuitRequestCallback_task::sendWakeupEvent(): Failed to send wakeup event" );
            }
        }
        
        #endif
    public:
        QuitRequestCallback_task()
        {
            // DEBUG:
            ff::write( jb_out, ">>> QuitRequestCallback_task()\n" );
            
            jade::scoped_lock< jade::mutex > slock( quit_mutex );               // Not strictly neccessary as this task is only created on a single thread
            quit_callback_pending = true;
        }
        bool execute( jade::task_mask* )
        {
            // DEBUG:
            ff::write( jb_out, ">>> QuitRequestCallback_task::execute()\n" );
            
            quit_mutex.lock();
            
            if( quit_callback )
                quit_callback -> call();
            
            quit_mutex.unlock();                                                // Allow any other threads possibly trying to cancel quitting to do so
            quit_mutex.lock();
            
            if( quit_flag )
            {
                cleanup_flag = true;
                quit_callback_pending = false;
                
                quit_mutex.unlock();
                
                // jade::stopEventSystem();
                
                jade::closeInputDevices();
                
                // LOLNOPE:
                sendWakeupEvent();
                jade::exit_code elc = event_loop.wait();
                // // HELLYES:
                // jade::scoped_lock< jade::mutex > klock( event_loop_kill_mutex );
                // event_loop.kill();
                // ff::write( jb_out, "Event loop killed\n" );
                
                {
                    ff::write( jb_out, "Cleaning up...\n" );
                    
                    if( cleanup_callback )
                        cleanup_callback -> call();
                    
                    jade::closeAllWindows();
                    
                    jade::deInitNamedResources();
                    
                    jade::stopTaskSystem();
                }
                
                if( elc != jade::EXITCODE_FINE )
                    ff::write( jb_out,
                               "Warning: Event loop exited with code ",
                               jade::exc2str( elc ),
                               "\n" );
            }
            else
            {
                quit_callback_pending = false;
                quit_mutex.unlock();
                ff::write( jb_out, "Quit interrupted\n" );
            }
            
            return true;
        }
        jade::task_mask getMask()
        {
            return jade::TASK_SYSTEM;
        }
    };
    
    // EVENT HANDLERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    void handleKeyEvent( XEvent& x_event )
    {
        jb_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        
        if( !jade::isRegisteredWindow( platform_window ) )
            throw jade::exception( "handleKeyEvent(): No such window registered" );
        
        jade::window& target_window( jade::getWindow( platform_window ) );
        
        jade::window_event w_event;
        w_event.type = jade::KEYCOMMAND;
        
        switch( x_event.xkey.type )
        {
            case KeyPress:
                w_event.key.up = false;
                break;
            case KeyRelease:
                w_event.key.up = true;
                break;
            default:
                throw jade::exception( "handleKeyEvent(): Key event neither up nor down" );
        }
        
        w_event.key.key = jade::convertPlatformKeycode( XLookupKeysym( &x_event.xkey, 0 ) );
        
        if( w_event.key.key != jade::KEY_INVALID )                              // Simply ignore invalid keys
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
            
            target_window.acceptEvent( w_event );
        }
    }
    
    void handleTextEvent( XEvent& x_event )
    {
        // http://www.x.org/releases/X11R7.6/doc/libX11/specs/XIM/xim.html
    }
    
    void handleWindowEvent( XEvent& x_event )
    {
        using namespace jade;
        
        jb_platform_window_t platform_window;
        platform_window.x_window = x_event.xany.window;
        
        try
        {
            jade::window& jb_window( getWindow( platform_window ) );            // Try block is for this statement, see comment in catch
            
            if( isRegisteredWindow( platform_window ) )
            {
                if( !window_manipulates.count( platform_window.x_window ) )
                    window_manipulates[ platform_window.x_window ] = new window::manipulate( &jb_window );
            }
            else
            {
                if( getDevMode() )
                    ff::write( jb_out, "X window event received for unregistered window, ignoring\n" );
                
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
                // current_manip -> makeActive();
                break;
            case ConfigureRequest:
                // x_event.xconfigurerequest
                break;
            case ConfigureNotify:
                {
                    jade::dpi::percent jb_window_scale = jb_window.getScaleFactor();
                    
                    auto jb_window_pos = jb_window.getPosition();
                    
                    dpi::points scaled_event_pos[ 2 ];
                    scaled_event_pos[ 0 ] = x_event.xconfigure.x / jb_window_scale;
                    scaled_event_pos[ 1 ] = x_event.xconfigure.y / jb_window_scale;
                    
                    // if( x_event.xconfigure.value_mask & CWX || x_event.xconfigure.value_mask & CWY )
                    if( scaled_event_pos[ 0 ] != jb_window_pos.first
                        || scaled_event_pos[ 1 ] != jb_window_pos.second )
                    {
                        current_manip -> setPosition( scaled_event_pos[ 0 ],
                                                      scaled_event_pos[ 1 ] );
                    }
                    
                    auto jb_window_dims = jb_window.getDimensions();
                    
                    dpi::points scaled_event_dims[ 2 ];
                    scaled_event_dims[ 0 ] = x_event.xconfigure.width  / jb_window_scale;
                    scaled_event_dims[ 1 ] = x_event.xconfigure.height / jb_window_scale;
                    
                    // if( x_event.xconfigure.value_mask & CWWidth || x_event.xconfigure.value_mask & CWHeight )
                    if( scaled_event_dims[ 0 ] != jb_window_dims.first
                        || scaled_event_dims[ 1 ] != jb_window_dims.second )
                    {
                        if( scaled_event_dims[ 0 ] < 1
                            || scaled_event_dims[ 1 ] < 1 )                     // Trust no one, not even your platform APIs
                            throw exception( "handleWindowEvent(): Width or height not within limits" );
                        
                        // ff::write( jb_out, "Setting dimensions\n" );
                        current_manip -> setDimensions( scaled_event_dims[ 0 ],
                                                        scaled_event_dims[ 1 ] );
                    }
                }
                break;
            case MapNotify:
            case MapRequest:
            case VisibilityNotify:
            case FocusIn:
                current_manip -> restore();
                // current_manip -> makeActive();
                break;
            case FocusOut:
                // current_manip -> makeInactive();
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
                    ff::write( jb_out, "Received unrecognized X window event, ignoring\n" );
                break;
            }
        }
        catch( exception& e )
        {
            if( x_event.type != FocusOut )                                      // Window close generates a FocusOut event, but window is already gone
                throw e;
            else
                if( getDevMode() )
                    ff::write( jb_out, "Got FocusOut X event for invalid window, assumed closed\n" );
        }
    }
    
    #endif
    
    // EVENT LOOP FUNCTION /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    #if defined PLATFORM_XWS_GNUPOSIX
    
    int x_event_queue_size = 0;
    
    jade::exit_code eventLoop( void* data )
    {
        // return EXIT_FINE;
        try
        {
            // TODO: Rework for DevicePresenceNotify
            jade::refreshInputDevices();                                                  // Set up input devices right off the bat
            
            XEvent x_event;
            Display* x_display = getXDisplay();
            
            XEvent last_x_dmevent;                                                  // For storing DeviceMotion events until we can fill out their window field
                                                                                    // Ideally we'd have a map of device ids -> XEvents in case we have multiple
                                                                                    // devices' worth of events waiting for windows, but MotionEvents don't include
                                                                                    // device information, so we just have to trust that X gives us events grouped
                                                                                    // together (DeviceMotion event will be followed by its MotionEvent before any
                                                                                    // other DeviceMotion events).
            bool dmevent_waiting = false;
            
            int dpne_type;
            XEventClass dpne_class;
            DevicePresence( x_display, dpne_type, dpne_class );
            
            // DEBUG:
            ff::write( jb_out, ">>> Hello\n" );
            
            while( true ) {
            // // while( !jade::isQuitting() )
            // while( x_event_queue_size )
            for( int x_event_queue_size = XEventsQueued( x_display, QueuedAfterFlush );
                 x_event_queue_size > 0;
                 ++x_event_queue_size )
            {
                // DEBUG:
                ff::write( jb_out, ">>> looking for event\n" );
                
                XNextEvent( x_display, &x_event );                                  // Blocks
                
                // jade::scoped_lock< jade::mutex > klock( event_loop_kill_mutex );
                
                // DEBUG:
                ff::write( jb_out, ">>> event~!\n" );
                
                // TODO: Rework for DevicePresenceNotify
                jade::refreshInputDevices();
                
                {
                    jade::scoped_lock< jade::mutex > slock( quit_mutex );
                    
                    if( quit_flag )
                    {
                        if( cleanup_flag )
                        {
                        //     ff::write( jb_out, "Cleaning up...\n" );
                            
                        //     if( cleanup_callback )
                        //         cleanup_callback -> call();
                            
                        //     jade::closeAllWindows();
                            
                        //     jade::deInitNamedResources();
                            
                        //     jade::stopTaskSystem();
                            
                        //     // DEBUG:
                        //     ff::write( jb_out, ">>> exiting event loop\n" );
                            
                            return EXIT_FINE;
                        }
                        else
                            if( !quit_callback_pending )
                            {
                                ff::write( jb_out, "Quitting...\n" );
                                
                                jade::submitTask( new QuitRequestCallback_task() );
                            }
                            
                    }
                }
                
                // if( x_event_queue_size == 0 )
                //     x_event_queue_size = XEventsQueued( x_display, QueuedAfterFlush );  // AKA XPending( x_display )
                
                switch( x_event.type )
                {
                case KeyRelease:
                    if( XEventsQueued( x_display, QueuedAfterReading ) )            // Skip key repeats
                    {
                        XEvent x_nextevent;
                        XPeekEvent( x_display, &x_nextevent );
                        
                        if( x_nextevent.type == KeyPress
                            && x_nextevent.xkey.time == x_event.xkey.time
                            && x_nextevent.xkey.keycode == x_event.xkey.keycode )   // Key repeat
                        {
                            XNextEvent( x_display, &x_event );                      // Get the repeated key press
                            XNextEvent( x_display, &x_event );                      // Get the event after the repeat
                        }
                    }                                                               // Fall through
                case KeyPress:
                    handleKeyEvent( x_event );
                    break;
                case ClientMessage:
                    // TODO: Handle/ignore Atom "jade::eventLoopWakeup"
                    if( x_event.xclient.message_type == wakeup_atom )
                        ff::write( jb_out,
                                   ">>> Got ClientMessage with wakeup atom\n" );
                    else
                        ff::write( jb_out,
                                   ">>> Got ClientMessage WITHOUT wakeup atom (likely WM event)\n" );
                case Expose:
                case ConfigureRequest:
                case ConfigureNotify:
                case MapNotify:
                case MapRequest:
                case VisibilityNotify:
                case FocusIn:
                case FocusOut:
                    ff::write( jb_out, ">>> handleWindowEvent()\n" );
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
                    break;                                                          // Ignore, for now
                case MotionNotify:
                    if( dmevent_waiting )
                    {
                        XDeviceMotionEvent& x_dmevent( *( ( XDeviceMotionEvent* )&last_x_dmevent ) );
                        
                        if( x_dmevent.time == x_event.xmotion.time )
                        {
                            x_dmevent.window = x_event.xmotion.window;              // Copy over window
                            
                            jade::handleStrokeEvent( last_x_dmevent );
                            
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
                            x_dmevent.window = x_event.xmotion.window;              // Copy over window
                            
                            jade::handleStrokeEvent( last_x_dmevent );
                            
                            dmevent_waiting = false;
                        }
                    }
                    break;
                default:
                    if( x_event.type == dpne_type )
                        // DEBUG:
                        ff::write( jb_out, ">>> Got XI_DevicePresenceNotify (holy shit!)\n" );
                    else
                        ff::write( jb_out, ">>> Got unrecognized event type\n" );
                    
                    last_x_dmevent = x_event;                                       // Save event so we can fill out the window field later
                    dmevent_waiting = true;
                    break;
                }
                
                --x_event_queue_size;
                
                // if( x_event_queue_size == 0 )
                {
                    for( auto iter = window_manipulates.begin();
                         iter != window_manipulates.end();
                         ++iter )
                    {
                        submitTask( iter -> second );
                    }
                    window_manipulates.clear();
                }
            } }
            
            // DEBUG:
            ff::write( jb_out, "Leaving event loop...\n" );
        }
        catch( jade::exception& e )
        {
            ff::write( jb_out, "jadebase exception from event loop: ", e.what(), "\n" );
            
            return jade::EXITCODE_JBERR;
        }
        catch( std::exception& e )
        {
            ff::write( jb_out, "Exception from event loop: ", e.what(), "\n" );
            
            return jade::EXITCODE_STDERR;
        }
        
        // DEBUG:
        ff::write( jb_out, "Exiting event loop\n" );
        
        return jade::EXITCODE_FINE;
    }
    
    #endif
}

/******************************************************************************//******************************************************************************/

#if defined PLATFORM_XWS_GNUPOSIX

namespace jade
{
    void requestQuit()
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        quit_flag = true;
        
        #if 0
        // Possible semi-blocking implementation
        if( !quit_flag )
        {
            jade::scoped_lock< jade::mutex > slock( quit_mutex );
            quit_flag = true;
        }
        #endif
    }
    void cancelQuit()
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        if( !cleanup_flag )
            quit_flag = false;
    }
    bool isQuitting()
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        return quit_flag;
    }
    bool isCleaningUp()
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        return cleanup_flag;
    }
    
    void setQuitRequestCallback( const std::shared_ptr< callback >& c )
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        quit_callback = c;
    }
    void setQuitCleanupCallback( const std::shared_ptr< callback >& c )
    {
        jade::scoped_lock< jade::mutex > slock( quit_mutex );
        cleanup_callback = c;
    }
    
    void startEventSystem()
    {
        #if defined PLATFORM_XWS_GNUPOSIX
        wakeup_atom = XInternAtom( getXDisplay(),
                                   "jade::eventLoopWakeup",
                                   0 );
        #endif
        
        event_loop.start( eventLoop );
    }
    void stopEventSystem()
    {
        // closeInputDevices();
        
        // // send wakeup
        
        // // wait on thread
        
        // // HELLYES:
        // jade::scoped_lock< jade::mutex > klock( event_loop_kill_mutex );
        // event_loop.kill();
        // ff::write( jb_out, "Event loop killed\n" );
    }
}

#else

#error "Events not implemented on non-X platforms"

#endif


