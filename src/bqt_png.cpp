/* 
 * bqt_png.cpp
 * 
 * Implements PNG utilities from bqt_png.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_png.hpp"

#include "bqt_exception.hpp"
#include "bqt_log.hpp"

/******************************************************************************//******************************************************************************/

#define PNG_HEADER_BYTES 8

namespace bqt
{
    png_file::png_file( std::string filename )
    {
        unsigned char   header[ PNG_HEADER_BYTES ];
        
        c_file = fopen( filename.c_str(), "rb" );
        
        if( c_file == NULL )
        {
            open = false;
            
            exception e;
            ff::write( *e, "png_file::png_file(): Could not open file \"", filename, "\"" );
            throw e;
        }
        
        fread( header, 1, PNG_HEADER_BYTES, c_file );                           // Read header
        
        if( png_sig_cmp( header, 0, PNG_HEADER_BYTES ) )
        {
            fclose( c_file );
            
            open = false;
            
            exception e;
            ff::write( *e, "png_file::png_file(): File \"", filename, "\" is not a PNG" );
            throw e;
        }
        
        file_struct = png_create_read_struct( PNG_LIBPNG_VER_STRING,
                                              NULL,
                                              NULL,
                                              NULL );                           // Create png_structp
        
        if( file_struct == NULL )
        {
            fclose( c_file );
            
            open = false;
            
            exception e;
            ff::write( *e, "png_file::png_file(): Could not create a png_struct for file \"", filename, "\"" );
            throw e;
        }
        
        file_info = png_create_info_struct( file_struct );
        
        if( file_info == NULL )
        {
            fclose( c_file );
            
            open = false;
            
            exception e;
            ff::write( *e, "png_file::png_file(): Could not create a png_info for file \"", filename, "\"" );
            throw e;
        }
        
        if( setjmp( png_jmpbuf( file_struct ) ) )                               // Set error jumpback point for reading
        {
            fclose( c_file );
            png_destroy_read_struct( &file_struct, &file_info, NULL );
            
            open = false;
            
            exception e;
            ff::write( *e, "png_file::png_file(): Error reading file \"", filename, "\"" );
            throw e;
        }
        
        png_init_io( file_struct, c_file );                                     // Read initialization
        
        png_set_sig_bytes( file_struct, PNG_HEADER_BYTES );
        
        png_read_png( file_struct,
                      file_info,
                      PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND,
                      NULL );                                                   // Read in entire PNG file
        
        dimensions[ 0 ] = png_get_image_width(  file_struct, file_info );
        dimensions[ 1 ] = png_get_image_height( file_struct, file_info );
        type = ( color_type )png_get_color_type( file_struct, file_info );
        bit_depth = png_get_bit_depth( file_struct, file_info );
        int interlace_type = png_get_interlace_type( file_struct, file_info );
        
        open = true;
    }
    png_file::~png_file()
    {
        if( open )
        {
            png_destroy_read_struct( &file_struct, &file_info, NULL );          // Clean up memory allocated by libpng
            
            fclose( c_file );                                                   // Clean up open file
        }
    }
    
    std::pair< unsigned int, unsigned int > png_file::getDimensions()
    {
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ],
                                                        dimensions[ 1 ] );
    }
    
    size_t png_file::getBitDepth()
    {
        return bit_depth;
    }
    
    png_file::color_type png_file::getColorType()
    {
        return type;
    }
    
    void png_file::toRGBABytes( unsigned char*& data )
    {
        if( !open )
            throw exception( "png_file::toRGBABytes(): File not open" );
        
        unsigned int bytes_per_row = png_get_rowbytes( file_struct, file_info );
        
        if( bytes_per_row / dimensions[ 0 ] != 4 )
            throw exception( "png_file::toRGBABytes(): Bytes per pixel != 4" );
        
        png_bytepp rows = png_get_rows( file_struct, file_info );
        
        for( int y = 0; y < dimensions[ 1 ]; ++y )
        {
            png_bytep row = rows[ y ];
            
            for( int x = 0; x < dimensions[ 0 ]; ++x )
            {
                data[ y * bytes_per_row + x * 4 + 0 ] = ( unsigned char )row[ x * 4 + 0 ];
                data[ y * bytes_per_row + x * 4 + 1 ] = ( unsigned char )row[ x * 4 + 1 ];
                data[ y * bytes_per_row + x * 4 + 2 ] = ( unsigned char )row[ x * 4 + 2 ];
                data[ y * bytes_per_row + x * 4 + 3 ] = ( unsigned char )row[ x * 4 + 3 ];
            }
        }
    }
}


