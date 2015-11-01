#ifndef JADEBASE_PNG_HPP
#define JADEBASE_PNG_HPP

/* 
 * jb_png.hpp
 * 
 * Utilities for loading & interpreting PNG files
 * 
 * png_file contains no thread-safety as it is read-only after construction (for
 * now).
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <cstdio>                                                               // libpng uses C-style file loading
#include <string>

#include <png.h>

#include "../utility/jb_dpi.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class png_file
    {
    public:                                                                     // Public types
        enum color_type
        {
            GRAY       = PNG_COLOR_TYPE_GRAY,
            PALETTE    = PNG_COLOR_TYPE_PALETTE,
            RGB        = PNG_COLOR_TYPE_RGB,
            RGB_ALPHA  = PNG_COLOR_TYPE_RGB_ALPHA,
            GRAY_ALPHA = PNG_COLOR_TYPE_GRAY_ALPHA
        };
        
        png_file( std::string );                                                // Opens the png file at the given path
        ~png_file();
        
        std::pair< dpi::pixels, dpi::pixels > getDimensions();
        
        size_t getBitDepth();
        
        color_type getColorType();
        
        void toRGBABytes( unsigned char*& );                                    // Assumes data has been allocated to w * h * 4 bytes
        
    protected:
        FILE* c_file;
        png_infop file_info;
        png_structp file_struct;
        
        color_type type;
        dpi::pixels dimensions[ 2 ];
        size_t bit_depth;
        
        bool open;
    };
}

/******************************************************************************//******************************************************************************/

#endif


