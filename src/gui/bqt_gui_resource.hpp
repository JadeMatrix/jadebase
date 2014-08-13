#ifndef BQT_GUI_RESOURCE_HPP
#define BQT_GUI_RESOURCE_HPP

/* 
 * bqt_gui_resource.hpp
 * 
 * Wrapper for bitmap GUI resources
 * 
 * gui_resource contains no thread-safety as after initialization it is read-
 * only.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "../bqt_gl.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    struct gui_texture
    {
        GLuint gl_texture;
        unsigned int dimensions[ 2 ];
        unsigned int ref_count;
        
        gui_texture( std::string f );
        ~gui_texture();
    };
    
    class gui_resource
    {
    protected:
        gui_texture* texture;
        unsigned int dimensions[ 2 ];                                           // Relative dimensions
        unsigned int   position[ 2 ];                                           // Relative position
        // bool custom;
    public:
        // gui_resource( std::string f );                                          // Loads file f if needed, using entire image for resource
        gui_resource( std::string f,                                            // Loads file f if needed, using subsection of image for resource
                      unsigned int x,
                      unsigned int y,
                      unsigned int w,
                      unsigned int h );
        ~gui_resource();                                                        // Cleans up image if needed
        
        std::pair< unsigned int, unsigned int > getDimensions();
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


