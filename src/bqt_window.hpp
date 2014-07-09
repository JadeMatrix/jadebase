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

// https://wiki.libsdl.org/SDL_WindowEventID?highlight=%28\bCategoryEnum\b%29|%28CategoryEvents%29
// https://wiki.libsdl.org/SDL_CreateSystemCursor
// needed? http://rainwarrior.ca/dragon/sdl_glsl.html
// http://www.opengl.org/wiki/Texture_Storage#Texture_copy

// Text input references:
// https://wiki.libsdl.org/Tutorials/TextInput
// http://lazyfoo.net/tutorials/SDL/32_text_input_and_clipboard_handling/index.php
// http://hg.libsdl.org/SDL/file/783d1cff9b20/test/testime.c

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_platform.h"
#include "bqt_canvas.hpp"
#include "bqt_task.hpp"
#include "bqt_mutex.hpp"
#include "bqt_version.hpp"
#include "bqt_event.hpp"

/******************************************************************************//******************************************************************************/

#define BQT_WINDOW_DEFAULT_NAME     BQT_VERSION_STRING
#define BQT_WINDOW_DEFAULT_WIDTH    256
#define BQT_WINDOW_DEFAULT_HEIGHT   256

namespace bqt
{
    typedef unsigned int window_id;
    typedef unsigned int view_id;
    typedef unsigned int tool_id;
    
    class window
    {
    private:
        bqt_platform_window_t platform_window;
        
        std::string title;
        int dimensions[2];
        int position[2];
        bool fullscreen;
        
        struct
        {
            bool changed    : 1;
            
            bool close      : 1;
            
            bool dimensions : 1;
            bool position   : 1;
            bool fullscreen : 1;
            bool title      : 1;
            bool minimize   : 1;
            bool maximize   : 1;
            bool restore    : 1;
        } updates;
        
        float view_zoom;
    protected:
        const window_id id;
        mutex window_mutex;
        
        void init();
        
        window( window_id id );
        ~window();
        
        void makeCurrent();
        
        // BQTDraw-specific stuff; exposed as higher-level functions ///////////
        
        void addCanvas( canvas* c, view_id v, int t );                         // Note: adding same canvas multiple times is safe (multiple views)
        void removeCanvas( canvas* c );
        
        // void splitView( view_id a, bool v );
        // void collapseViews( view_id a, view_id b );
        
        void setToolVisibility( bool v );
        void setViewZoom( view_id v, float z );
    public:
        std::pair< unsigned int, unsigned int > getDimensions();
        std::pair< unsigned int, unsigned int > getPosition();
        
        float getViewZoom( view_id v );
        
        void acceptEvent( event& e );
        
        class manipulate : public task
        {
        protected:
            window* target;
        public:
            manipulate( window* t = NULL );                                     // If target is NULL, injects a new window into the manager
            
            bool execute( task_mask* caller_mask );
            virtual task_mask getMask()
            {
                return TASK_SYSTEM;
            }
            
            void setDimensions( unsigned int w, unsigned int h );
            void setPosition( unsigned int x, unsigned int y );
            
            void setFullscreen( bool f );
            void setTitle( std::string t );
            
            void minimize();
            void maximize();
            void restore();
            void close();
            
            void dropCanvas( canvas* c, unsigned int x, unsigned int y );
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


