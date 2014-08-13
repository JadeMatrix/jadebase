/* 
 * bqt_gui_resource.cpp
 * 
 * Implements bqt_gui_resource & handles resource garbage collection
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource.hpp"

#include <map>
#include <cstdio>                                                               // libpng uses C-style file loading

#include "../bqt_mutex.hpp"
#include "../bqt_log.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_launchargs.hpp"
#include "../bqt_png.hpp"
#include "../bqt_taskexec.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex textures_mutex;
    std::map< std::string, bqt::gui_texture* > textures;
    
    bqt::gui_texture* acquireTexture( std::string f )
    {
        bqt::scoped_lock slock( textures_mutex );
        
        bqt::gui_texture* texture;                                              // Temp variable so we only access textures twice per call
        
        if( textures.count( f ) == 0 )
        {
            texture = new bqt::gui_texture( f );
            textures[ f ] = texture;
        }
        else
        {
            texture = textures[ f ];
            texture -> ref_count++;
        }
        
        return texture;
    }
    void releaseTexture( bqt::gui_texture* t )
    {
        bqt::scoped_lock slock( textures_mutex );
        
        for( std::map< std::string, bqt::gui_texture* >::iterator iter = textures.begin();
             iter != textures.end();
             ++iter )
        {
            if( iter -> second == t )
            {
                iter -> second -> ref_count--;
                
                if( iter -> second -> ref_count < 1 )
                {
                    delete t;
                    textures.erase( iter );
                }
                
                return;
            }
        }
        
        if( bqt::getDevMode() )
            ff::write( bqt_out, "Warning: releaseTexture(): Could not find texture to release\n" );
        else
            throw bqt::exception( "releaseTexture(): Could not find texture to release" );
    }
    
    class LoadGUIResource_task : public bqt::task
    {                                                                           // LoadGUIResource_task executes twice, once to load the PNG file and a second
                                                                                // time to convert the contents & upload to the GPU
    protected:
        std::string filename;
        unsigned char* data;
        unsigned int dimensions[ 2 ];
    public:
        LoadGUIResource_task( std::string f )
        {
            filename = f;
            data = NULL;
        }
        
        bool execute( bqt::task_mask* caller_mask )
        {
            if( data == NULL )
            {
                bqt::png_file rsrc_file( filename );
                
                std::pair< unsigned int, unsigned int > rsrc_dim = rsrc_file.getDimensions();
                
                data = new unsigned char[ rsrc_dim.first * rsrc_dim.second * 4 ];
                
                if( data == NULL )
                    throw bqt::exception( "LoadGUIResource_task::execute(): Could not allocate swap space" );
                
                rsrc_file.toRGBABytes( data );
                
                dimensions[ 0 ] = rsrc_dim.first;
                dimensions[ 1 ] = rsrc_dim.second;
                
                return false;                                                   // Requeue
            }
            else
            {
                {
                    bqt::scoped_lock slock( textures_mutex );
                    
                    if( textures.count( filename ) == 0 )                       // We're too late to load, the resource is no longer needed
                        return true;
                }
                
                GLuint gl_texture;
                
                glGenTextures( 1, &gl_texture );
                glBindTexture( GL_TEXTURE_2D, gl_texture );
                glTexImage2D( GL_TEXTURE_2D,
                              0,
                              GL_RGBA,
                              dimensions[ 1 ],
                              dimensions[ 0 ],
                              0,
                              GL_RGBA,
                              GL_UNSIGNED_BYTE,
                              data );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                
                glBindTexture( GL_TEXTURE_2D, 0 );
                
                GLenum gl_error = glGetError();
                if( gl_error != GL_NO_ERROR )
                {
                    bqt::exception e;
                    ff::write( *e,
                               "LoadGUIResource_task::execute(): OpenGL error: ",
                               "...",
                               " loading pixels from 0x",
                               ff::to_x( ( unsigned long )data, HEX_WIDTH, HEX_WIDTH ),
                               " to texture 0x",
                               ff::to_x( gl_texture, HEX_WIDTH, HEX_WIDTH ) );
                    throw e;
                }
                
                delete[] data;
                data = NULL;
                
                {
                    bqt::scoped_lock slock( textures_mutex );
                    
                    textures[ filename ] -> gl_texture = gl_texture;
                    textures[ filename ] -> dimensions[ 0 ] = dimensions[ 0 ];
                    textures[ filename ] -> dimensions[ 1 ] = dimensions[ 1 ];
                }
                
                return true;
            }
        }
        bqt::task_mask getMask()
        {
            if( data == NULL )
                return bqt::TASK_SYSTEM;                                        // libpng only works from the main thread
            else
                return bqt::TASK_GPU;
        }
    };
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    /* GUI_TEXTURE ************************************************************//******************************************************************************/
    
    gui_texture::gui_texture( std::string f )
    {
        gl_texture = 0x00;
        
        dimensions[ 0 ] = 0;
        dimensions[ 1 ] = 0;
        
        ref_count = 1;
        
        submitTask( new LoadGUIResource_task( f ) );
    }
    gui_texture::~gui_texture()
    {
        if( ref_count > 0 && getDevMode() )
            ff::write( bqt_out, "Warning: destroying bqt::gui_texture with ref_coutn > 0" );
        
        if( gl_texture != 0 )                                                   // Needed?
            glDeleteTextures( 1, &gl_texture );
    }
    
    /* GUI_RESOURCE ***********************************************************//******************************************************************************/
    
    gui_resource::gui_resource( std::string f,
                                unsigned int x,
                                unsigned int y,
                                unsigned int w,
                                unsigned int h )
    {
        texture = acquireTexture( f );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        releaseTexture( texture );
    }
    
    std::pair< unsigned int, unsigned int > gui_resource::getDimensions()
    {
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    
    void gui_resource::draw()
    {
        float draw_pos[ 2 ];
        float draw_dim[ 2 ];
        
        {
            scoped_lock slock( textures_mutex );                                // Needed?
            
            if( texture -> gl_texture == 0x00 )                                 // Resource not yet loaded
                return;
            else
            {                                                                   // TODO: Inefficient, find another way
                draw_pos[ 0 ] = (float )position[ 0 ] / texture -> dimensions[ 0 ];
                draw_pos[ 1 ] = (float )position[ 1 ] / texture -> dimensions[ 1 ];
                
                draw_dim[ 0 ] = (float )dimensions[ 0 ] / texture -> dimensions[ 0 ];
                draw_dim[ 1 ] = (float )dimensions[ 1 ] / texture -> dimensions[ 1 ];
            }
        }
        
        glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
        
        glBegin( GL_QUADS );
        {
            glTexCoord2f( draw_pos[ 0 ], draw_pos[ 1 ] );
            glVertex2f( 0, 0 );
            
            glTexCoord2f( draw_pos[ 0 ] + draw_dim[ 0 ], draw_pos[ 1 ] );
            glVertex2f( dimensions[ 0 ], 0 );
            
            glTexCoord2f( draw_pos[ 0 ] + draw_dim[ 0 ], draw_pos[ 1 ] + draw_dim[ 1 ] );
            glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
            
            glTexCoord2f( draw_pos[ 0 ], draw_pos[ 1 ] + draw_dim[ 1 ] );
            glVertex2f( 0, dimensions[ 1 ] );
        }
        glEnd();
    }
}


