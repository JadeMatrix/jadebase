/* 
 * bqt_gui_resource.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource.hpp"

#include "../bqt_launchargs.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    gui_resource::gui_resource( window& parent,
                                std::string f,
                                unsigned int x,
                                unsigned int y,
                                unsigned int w,
                                unsigned int h ) : parent( parent )
    {
        texture = parent.acquireTexture( f );
        
        rect_is_converted = false;
        
        rect.i.position[ 0 ] = x;
        rect.i.position[ 1 ] = y;
        
        rect.i.dimensions[ 0 ] = w;
        rect.i.dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        parent.releaseTexture( texture );
    }
    
    std::pair< unsigned int, unsigned int > gui_resource::getDimensions()
    {
        if( rect_is_converted )
            return std::pair< unsigned int, unsigned int >( rect.f.dimensions[ 0 ], rect.f.dimensions[ 1 ] );
        else
            return std::pair< unsigned int, unsigned int >( rect.i.dimensions[ 0 ], rect.i.dimensions[ 1 ] );
    }
    
    void gui_resource::draw()
    {
        if( texture -> gl_texture == 0x00 )                                     // Resource not yet loaded
        {
            if( getDevMode() )
            {
                glBegin( GL_QUADS );                                            // Using rect.i.* as conversion shouldn't have happened yet
                {
                    ff::write( bqt_out, "Drawing temp graphic\n" );
                    
                    glColor4f( 0.0, 1.0, 1.0, 1.0 );
                    glVertex2f( 0, 0 );
                    
                    glColor4f( 1.0, 0.0, 1.0, 1.0 );
                    glVertex2f( rect.i.dimensions[ 0 ], 0 );
                    
                    glColor4f( 1.0, 1.0, 0.0, 1.0 );
                    glVertex2f( rect.i.dimensions[ 0 ], rect.i.dimensions[ 1 ] );
                    
                    glColor4f( 0.0, 0.0, 0.0, 1.0 );
                    glVertex2f( 0, rect.i.dimensions[ 1 ] );
                }
                glEnd();
            }
            
            return;
        }
        
        if( !rect_is_converted )
        {
            float converted[ 4 ];
            
            converted[ 0 ] = ( float )rect.i.position  [ 0 ] / texture -> dimensions[ 1 ];
            converted[ 1 ] = ( float )rect.i.position  [ 1 ] / texture -> dimensions[ 0 ];
            converted[ 2 ] = ( float )rect.i.dimensions[ 0 ] / texture -> dimensions[ 1 ];
            converted[ 3 ] = ( float )rect.i.dimensions[ 1 ] / texture -> dimensions[ 0 ];
            
            rect_is_converted = true;
            
            rect.f.position  [ 0 ] = converted[ 0 ];
            rect.f.position  [ 1 ] = converted[ 1 ];
            rect.f.dimensions[ 0 ] = converted[ 2 ];
            rect.f.dimensions[ 1 ] = converted[ 3 ];
        }
        
        glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
        
        glPushMatrix();
        {
            glScalef( texture -> dimensions[ 0 ], texture -> dimensions[ 1 ], 1.0f );
            // glScalef( 256, 256, 1 );
            
            glBegin( GL_QUADS );
            {
                glTexCoord2f( rect.f.position[ 0 ], rect.f.position[ 1 ] );
                glVertex2f( 0, 0 );
                
                glTexCoord2f( rect.f.position[ 0 ], rect.f.position[ 1 ] + rect.f.dimensions[ 1 ] );
                glVertex2f( 0, rect.f.dimensions[ 1 ] );
                
                glTexCoord2f( rect.f.position[ 0 ] + rect.f.dimensions[ 0 ], rect.f.position[ 1 ] + rect.f.dimensions[ 1 ] );
                glVertex2f( rect.f.dimensions[ 0 ], rect.f.dimensions[ 1 ] );
                
                glTexCoord2f( rect.f.position[ 0 ] + rect.f.dimensions[ 0 ], rect.f.position[ 1 ] );
                glVertex2f( rect.f.dimensions[ 0 ], 0 );
            }
            glEnd();
        }
        glPopMatrix();
    }
}


