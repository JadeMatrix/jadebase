#ifndef BQT_WINDOW_HPP
#define BQT_WINDOW_HPP

/* 
 * bqt_window.hpp
 * 
 * Windows in BQTDraw are fairly, as they need to be
 * capable of wrapping and abstracting a large amount of platform-specific code,
 * or, in the case that functionality is not available, recreating it.
 * 
 * Canvases can be moved from one window to another (as tabs); this is handled
 * by the window being moved FROM.  It removes the document pointer from itself
 * and calls dropCanvas() on a manipulator for the target window.
 * 
 */

// http://www.opengl.org/wiki/Texture_Storage#Texture_copy

// http://tronche.com/gui/x/xlib/pixmap-and-cursor/cursor.html
// http://tronche.com/gui/x/xlib/window/XDefineCursor.html

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_platform.h"
#include "bqt_canvas.hpp"
#include "bqt_task.hpp"
#include "bqt_mutex.hpp"
#include "bqt_version.hpp"
#include "bqt_windowevent.hpp"
// #include "gui/bqt_layout.hpp"

/******************************************************************************//******************************************************************************/

#define BQT_WINDOW_DEFAULT_NAME     BQT_VERSION_STRING
#define BQT_WINDOW_DEFAULT_WIDTH    256
#define BQT_WINDOW_DEFAULT_HEIGHT   256
#define BQT_WINDOW_MIN_WIDTH        256
#define BQT_WINDOW_MIN_HEIGHT       256

namespace bqt
{
    class window
    {
    private:
        bqt_platform_window_t platform_window;
        
        unsigned int pending_redraws;
        
        std::string title;
        unsigned int dimensions[2];
        int position[2];
        bool fullscreen;
        bool in_focus;
        
        struct
        {
            bool changed    : 1;
            
            bool close      : 1;
            
            bool active     : 1;
            
            bool dimensions : 1;
            bool position   : 1;
            bool fullscreen : 1;
            bool title      : 1;
            bool center     : 1;
            bool minimize   : 1;
            bool maximize   : 1;
            bool restore    : 1;
        } updates;
        
        float view_zoom;
    protected:
        // layout gui;
        
        mutex window_mutex;
        void init();
        
        ~window();
        
        class redraw : public task
        {
        protected:
            window& target;
        public:
            redraw( window& t );
            bool execute( task_mask* caller_mask );
            task_priority getPriority()
            {
                return PRIORITY_HIGH;
            }
            task_mask getMask()
            {
                return TASK_GPU;
            }
        };
    public:
        window();
        
        std::pair< unsigned int, unsigned int > getDimensions();
        std::pair< int, int > getPosition();
        
        void acceptEvent( window_event& e );
        
        bqt_platform_window_t& getPlatformWindow();                             // TODO: make this const-correct
        
        void makeContextCurrent();                                              // Utility for TASK_GPU stuff
        
        class manipulate : public task
        {
            // TODO: Consider overriding new/delete for manipulates so we only
            // have one per window at any given time
        protected:
            window* target;
        public:
            manipulate( window* t = NULL );                                     // If target is NULL, injects a new window into the manager
            
            bool execute( task_mask* caller_mask );
            task_mask getMask()
            {
                return TASK_SYSTEM;
            }
            
            void setDimensions( unsigned int w, unsigned int h );
            void setPosition( int x, int y );
            
            void setFullscreen( bool f );
            void setTitle( std::string t );
            
            // void setFocus( bool f );                                            // Change any window styles to fit in- or out-of-focus
            void makeActive();
            
            void center();
            void minimize();
            void maximize();
            void restore();
            void close();
            
            void redraw();                                                      // Just redraw the window (sets changed flag to true)
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


