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
#include <memory>
#include <string>
#include <vector>

#include "jb_windowevent.hpp"
#include "../gui/jb_group.hpp"
#include "../tasking/jb_task.hpp"
#include "../threading/jb_mutex.hpp"
#include "../utility/jb_container.hpp"
#include "../utility/jb_platform.h"
#include "../utility/jb_version.hpp"

/******************************************************************************//******************************************************************************/

#define JADEBASE_WINDOW_DEFAULT_NAME     jade::getProgramVersionString()
#define JADEBASE_WINDOW_MIN_WIDTH        256
#define JADEBASE_WINDOW_MIN_HEIGHT       256

#define JADEBASE_WINDOW_CONTAINERREGISTERSILENTFAIL                             // That's kind of long

namespace jade
{
    class gui_element;
    class gui_resource;
    
    class window
    {
        friend class container< window >;
        
    public:
        window();
        
        std::pair< unsigned int, unsigned int > getDimensions();
        std::pair< int, int > getPosition();
        
        void acceptEvent( window_event& );
        
        jb_platform_window_t& getPlatformWindow();                              // TODO: make this const-correct
        
        // FIXME: What happens if the GUI layout changes and the offset to the capturing element needs updating, huh? (Gonna get ugly)
        void associateDevice( jb_platform_idevid_t,                             // ID of the device to associate
                              gui_element*,                                     // Pointer to the capturing element
                              float,                                            // X event offset
                              float );                                          // Y event offset
                                                                                // Begins sending input events from the device directly to the element without
                                                                                // passing through the element tree, using the given event offsets.
        void deassociateDevice( jb_platform_idevid_t dev_id );                  // Called when an association is no longer necessary; elements must deassociate
                                                                                // all associated devices before destruction.
        
        std::string getTitle();
        
        std::shared_ptr< group > getTopGroup();                                 // Get top-level GUI group element (returns std::shared_ptr for integration with
                                                                                // GCed scripting languages)
        
        void register_container( container< window >* );
        void deregister_container( container< window >* );
        
        void requestRedraw();
        
        class manipulate : public task                                          // TODO: Rename to ManipulateWindow_task
        {
            // TODO: Consider overriding new/delete for manipulates so we only
            // have one per window at any given time
        public:
            manipulate( window* t = NULL );                                     // If target is NULL, injects a new window into the manager
            
            bool execute( task_mask* );
            task_mask getMask()
            {
                return TASK_SYSTEM;
            }
            
            void setDimensions( unsigned int, unsigned int );                   // Width, height
            void setPosition( int, int );                                       // X, Y
            
            void setFullscreen( bool );
            void setTitle( std::string );
            
            // void setFocus( bool );                                              // Change any window styles to display in- or out-of-focus
            
            void center();
            void minimize();
            void maximize();
            void restore();
            void close();
            
            void redraw();                                                      // Just (ask to) redraw the window (sets changed flag to true)
            
        protected:
            window* target;
        };
        
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
        std::shared_ptr< group > top_group;
        
        /* Container infrastructure *******************************************//******************************************************************************/
        
        bool can_add_containers;
        std::set< container< window >* > containers;
        
        /**********************************************************************//******************************************************************************/
        
        void init();
        
        void makeContextCurrent();                                              // Not thread-safe
        
        ~window();                                                              // Windows can only be destroyed by manipulate tasks
        
        class redraw : public task                                              // TODO: Rename to RedrawWindow_task
        {
        public:
            redraw( window& );
            bool execute( task_mask* );
            task_priority getPriority()
            {
                return PRIORITY_HIGH;
            }
            task_mask getMask()
            {
                return TASK_GPU;
            }
            
        protected:
            window& target;
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


