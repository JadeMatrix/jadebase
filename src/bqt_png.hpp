#ifndef BQT_PNG_HPP
#define BQT_PNG_HPP

/* 
 * bqt_png.hpp
 * 
 * Utilities for loading & interpreting PNG files
 * 
 * png_file contains no thread-safety as it is read-only after construction (for
 * now).
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>
#include <cstdio>                                                               // libpng uses C-style file loading

#include <png.h>

/******************************************************************************//******************************************************************************/

namespace bqt
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
    protected:
        FILE* c_file;
        png_infop file_info;
        png_structp file_struct;
        
        color_type type;
        unsigned int dimensions[ 2 ];
        size_t bit_depth;
        
        bool open;
    public:
        png_file( std::string filename );
        ~png_file();
        
        std::pair< unsigned int, unsigned int > getDimensions();
        
        size_t getBitDepth();
        
        color_type getColorType();
        
        void toRGBABytes( unsigned char*& data );                               // Assumes data has been allocated to w * h * 4 bytes
    };
}

/******************************************************************************//******************************************************************************/

#endif


