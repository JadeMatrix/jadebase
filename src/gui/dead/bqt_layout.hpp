#ifndef BQT_LAYOUT_HPP
#define BQT_LAYOUT_HPP

/* 
 * bqt_layout.hpp
 * 
 * GUI layout handling
 * 
 * Even though GUIs are by definition locked to a window, layout is thread-safe
 * for any future needs.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>
#include <map>

#include "../bqt_windowevent.hpp"
#include "../bqt_mutex.hpp"
#include "../bqt_task.hpp"
#include "bqt_gui_resource_names.hpp"
#include "bqt_gui_texture.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class window;                                                               // Defined in bqt_window.hpp
    class layout_element;                                                       // Defined in bqt_layout_element.hpp
    class gui_resource;                                                         // Defined in bqt_gui_resource.hpp
    
    class layout
    {
        friend class gui_resource;
    protected:
        mutex layout_mutex;
        window* parent;                                                         // Unfortunately needed for getting OpenGL context
        
        unsigned int dimensions[ 2 ];
        
        std::vector< layout_element* > elements;
        
        std::map< bqt_platform_idevid_t, layout_element* > input_assoc;
        
        struct gui_texture_wrapper
        {
            gui_texture* texture;
            
            unsigned char* data;
            int ref_count;
            
            gui_texture_wrapper()
            {
                texture = new gui_texture();
                data = NULL;
                ref_count = 0;
            }
            ~gui_texture_wrapper()
            {
                if( data != NULL )
                    delete[] data;
                delete texture;
            }
        };
        
        std::map< std::string, gui_texture_wrapper* > resource_textures;
        std::map< gui_resource_name, gui_resource* > named_resources;
    public:
        layout( window* p, unsigned int w, unsigned int h );
        ~layout();
        
        void initNamedResources();
        
        std::pair< unsigned int, unsigned int > getDimensions();
        void setDimensions( unsigned int w, unsigned int h );
        
        // bool addNamedResource( gui_resource_name n, gui_resource* r );
        
        gui_texture* acquireTexture( std::string f );
        void releaseTexture( gui_texture* t );
        
        gui_resource* getNamedResource( gui_resource_name name );
        
        void associateDevice( bqt_platform_idevid_t dev_id,
                              layout_element* element );                        // Begins sending input events from the device directly to the element without
                                                                                // passing through the element tree; deassociates if element is NULL.
        
        void acceptEvent( window_event& e );
        
        void draw();
        
        void startClean();                                                      // Deletes all elements & resources; safe to call multiple times
        bool isClean();
        
        class InitLayoutResources_task : public task
        {
        protected:
            layout& target;
            enum
            {
                OPENING_FILES,
                UPLOADING_TEXTURES
            } state;
        public:
            InitLayoutResources_task( layout& t );
            
            bool execute( task_mask* caller_mask );
            task_mask getMask();
        };
        
        class DeinitLayoutResources_task : public task
        {
        protected:
            layout& target;
        public:
            DeinitLayoutResources_task( layout& t );
            
            bool execute( task_mask* caller_mask );
            task_mask getMask();
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


