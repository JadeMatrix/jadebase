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
#include <map>
#include <vector>

#include "bqt_platform.h"
#include "bqt_canvas.hpp"
#include "bqt_task.hpp"
#include "threading/bqt_rwlock.hpp"
#include "bqt_version.hpp"
#include "bqt_windowevent.hpp"
#include "gui/bqt_gui_texture.hpp"
#include "gui/bqt_gui_resource_names.hpp"

/******************************************************************************//******************************************************************************/

#define BQT_WINDOW_DEFAULT_NAME     BQT_VERSION_STRING
#define BQT_WINDOW_DEFAULT_WIDTH    256
#define BQT_WINDOW_DEFAULT_HEIGHT   256
#define BQT_WINDOW_MIN_WIDTH        256
#define BQT_WINDOW_MIN_HEIGHT       256

namespace bqt
{
    class gui_element;
    class gui_resource;
    
    class window
    {
    protected:
        rwlock window_lock;
        
        /* Window infrastructure **********************************************//******************************************************************************/
        
        bqt_platform_window_t platform_window;
        
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
            
            bool active     : 1;
            
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
        
        std::map< bqt_platform_idevid_t,
                  gui_element*,
                  bool ( * )( const bqt_platform_idevid_t&, const bqt_platform_idevid_t& ) > input_assoc;
        std::vector< gui_element* > elements;
        
        struct gui_texture_holder
        {
            gui_texture* texture;
            
            unsigned char* data;
            int ref_count;
            
            gui_texture_holder()
            {
                texture = new gui_texture();
                data = NULL;
                ref_count = 0;
            }
            ~gui_texture_holder()
            {
                delete texture;
            }
        };
        
        std::map< std::string, gui_texture_holder* > resource_textures;
        bool new_textures;
        bool old_textures;
        
        std::map< gui_resource_name, gui_resource* > named_resources;
        
        /**********************************************************************//******************************************************************************/
        
        void init();
        
        void makeContextCurrent();                                              // Not thread-safe
        
        void initNamedResources();
        
        void openUnopenedTextureFiles();
        void uploadUnuploadedTextures();
        void deleteUnreferencedTextures();
        
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
        
        bqt_platform_window_t& getPlatformWindow();                             // TODO: make this const-correct
        
        void associateDevice( bqt_platform_idevid_t dev_id,
                              gui_element* element );                           // Begins sending input events from the device directly to the element without
                                                                                // passing through the element tree; deassociates if element is NULL.
        
        gui_texture* acquireTexture( std::string filename );
        void releaseTexture( gui_texture* t );
        
        gui_resource* getNamedResource( gui_resource_name name );
        
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


