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

/******************************************************************************//******************************************************************************/

namespace bqt
{
    gui_resource::gui_resource( layout& p,
                                std::string f,
                                unsigned int x,
                                unsigned int y,
                                unsigned int w,
                                unsigned int h ) : parent( p )
    {
        ff::write( bqt_out, "Creating gui_resource \"", f, "\" ", x, " ", y, " ", w, " ", h, "\n" );
        
        texture = parent.acquireTexture( f );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        parent.releaseTexture( texture );
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
            // TODO: thread safety for accessing texture here?  texture::gl_texture should not be written to on any other thread though
            
            if( texture -> gl_texture == 0x00 )                                 // Resource not yet loaded
            {
                if( getDevMode() )
                {
                    ff::write( bqt_out, "Drawing placeholder graphic\n" );
                    glBegin( GL_QUADS );
                    {
                        glColor4f( 0.0, 1.0, 1.0, 1.0 );
                        glVertex2f( 0, 0 );
                        
                        glColor4f( 1.0, 0.0, 1.0, 1.0 );
                        glVertex2f( dimensions[ 0 ], 0 );
                        
                        glColor4f( 1.0, 1.0, 0.0, 1.0 );
                        glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                        
                        glColor4f( 0.0, 0.0, 0.0, 1.0 );
                        glVertex2f( 0, dimensions[ 1 ] );
                    }
                    glEnd();
                }
                else
                    return;
            }
            else
            {                                                                   // TODO: Inefficient, find another way
                ff::write( bqt_out,
                           "Drawing with texture 0x",
                           ff::to_x( texture -> gl_texture ),
                           "\n" );
                
                draw_pos[ 0 ] = (float )position[ 0 ] / texture -> dimensions[ 0 ];
                draw_pos[ 1 ] = (float )position[ 1 ] / texture -> dimensions[ 1 ];
                
                draw_dim[ 0 ] = (float )dimensions[ 0 ] / texture -> dimensions[ 0 ];
                draw_dim[ 1 ] = (float )dimensions[ 1 ] / texture -> dimensions[ 1 ];
            }
        }
        
        glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
        
        glBegin( GL_QUADS );
        {
            // glTexCoord2f( draw_pos[ 0 ], draw_pos[ 1 ] );
            glColor4f( 0.0, 1.0, 1.0, 1.0 );
            glVertex2f( 0, 0 );
            
            // glTexCoord2f( draw_pos[ 0 ] + draw_dim[ 0 ], draw_pos[ 1 ] );
            glColor4f( 1.0, 0.0, 1.0, 1.0 );
            glVertex2f( dimensions[ 0 ], 0 );
            
            // glTexCoord2f( draw_pos[ 0 ] + draw_dim[ 0 ], draw_pos[ 1 ] + draw_dim[ 1 ] );
            glColor4f( 1.0, 1.0, 0.0, 1.0 );
            glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
            
            // glTexCoord2f( draw_pos[ 0 ], draw_pos[ 1 ] + draw_dim[ 1 ] );
            glColor4f( 0.0, 0.0, 0.0, 1.0 );
            glVertex2f( 0, dimensions[ 1 ] );
        }
        glEnd();
    }
}


