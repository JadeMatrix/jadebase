#ifndef JADEBASE_WINDOW_HPP
#define JADEBASE_WINDOW_HPP

/* 
 * jb_window.hpp
 * 
 * Windows in jadebase are fairly monolithic, as they need to be capable of
 * wrapping and abstracting a large amount of platform-specific code, or, in the
 * case that functionality is not available, recreating it.
 * 
 * Moving data from one window to another (such as document views) is handled by
 * the window being moved FROM.  It removes the data/pointer from itself and
 * then passes it to the target window.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <map>
#include <string>
#include <vector>

#include "jb_windowevent.hpp"
#include "../tasking/jb_task.hpp"
#include "../threading/jb_mutex.hpp"
#include "../utility/jb_platform.h"
#include "../utility/jb_version.hpp"

/******************************************************************************//******************************************************************************/

#define JADEBASE_WINDOW_DEFAULT_NAME     jade::getProgramVersionString()
#define JADEBASE_WINDOW_MIN_WIDTH        256
#define JADEBASE_WINDOW_MIN_HEIGHT       256

namespace jade
{
    class gui_element;
    class gui_resource;
    class group;
    
    class window
    {
    protected:
        mutex window_mutex;
        
        /* Window infrastructure **********************************************//******************************************************************************/
        
        jb_platform_window_t platform_window;
        
        std::string title;
        unsigned int dimensions[2];
        int position[2];
        bool fullscreen;
        bool in_focus;
        
        unsigned int pending_redraws;
        
        struct
        {
            bool changed    : 1;
            
            bool close      : 1;
            
            bool dimensions : 1;
            bool position   : 1;
            bool fullscreen : 1;
            bool title      : 1;
            bool center     : 1;
            bool minimize   : 1;
            bool maximize   : 1;
            bool restore    : 1;
            
            bool redraw     : 1;
        } updates;
        
        /* GUI infrastructure *************************************************//******************************************************************************/
        
        struct idev_assoc
        {
            gui_element* element;
            float offset[ 2 ];
        };
        
        std::map< jb_platform_idevid_t,
                  idev_assoc > input_assoc;
        group* top_group;
        
        /**********************************************************************//******************************************************************************/
        
        void init();
        
        void makeContextCurrent();                                              // Not thread-safe
        
        ~window();                                                              // Windows can only be destroyed by manipulate tasks
        
        class redraw : public task                                              // TODO: Rename to RedrawWindow_task
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
        
        jb_platform_window_t& getPlatformWindow();                              // TODO: make this const-correct
        
        void associateDevice( jb_platform_idevid_t dev_id,
                              gui_element* element,
                              float off_x,
                              float off_y );                                    // Begins sending input events from the device directly to the element without
                                                                                // passing through the element tree.
        void deassociateDevice( jb_platform_idevid_t dev_id );                  // Called when an association is no longer necessary; elements must deassociate
                                                                                // all associated devices before destruction.
        
        std::string getTitle();
        
        group* getTopGroup();                                                   // Get top-level GUI group element
        
        void requestRedraw();
        
        class manipulate : public task                                          // TODO: Rename to ManipulateWindow_task
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


