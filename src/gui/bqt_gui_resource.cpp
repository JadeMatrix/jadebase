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
    void gui_resource::createDisplayList()
    {
        float f_position  [ 2 ];
        float f_dimensions[ 2 ];
        
        f_position  [ 0 ] = ( float )position  [ 0 ] / texture -> dimensions[ 1 ];
        f_position  [ 1 ] = ( float )position  [ 1 ] / texture -> dimensions[ 0 ];
        f_dimensions[ 0 ] = ( float )dimensions[ 0 ] / texture -> dimensions[ 1 ];
        f_dimensions[ 1 ] = ( float )dimensions[ 1 ] / texture -> dimensions[ 0 ];
        
        gl_dlist = glGenLists( 1 );
        
        glNewList( gl_dlist, GL_COMPILE );
        {
            glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
            
            glBegin( GL_QUADS );
            {
                glTexCoord2f( f_position[ 0 ], f_position[ 1 ] );
                glVertex2f( 0, 0 );
                
                glTexCoord2f( f_position[ 0 ], f_position[ 1 ] + f_dimensions[ 1 ] );
                glVertex2f( 0, dimensions[ 1 ] );
                
                glTexCoord2f( f_position[ 0 ] + f_dimensions[ 0 ], f_position[ 1 ] + f_dimensions[ 1 ] );
                glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                
                glTexCoord2f( f_position[ 0 ] + f_dimensions[ 0 ], f_position[ 1 ] );
                glVertex2f( dimensions[ 0 ], 0 );
            }
            glEnd();
        }
        glEndList();
        
        dlist_created = true;
    }
    
    gui_resource::gui_resource( window& parent,
                                std::string f,
                                unsigned int x,
                                unsigned int y,
                                unsigned int w,
                                unsigned int h ) : parent( parent )
    {
        texture = parent.acquireTexture( f );
        
        dlist_created = false;
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    gui_resource::~gui_resource()
    {
        if( dlist_created )
            glDeleteLists( gl_dlist, 1 );
        
        parent.releaseTexture( texture );
    }
    
    std::pair< unsigned int, unsigned int > gui_resource::getDimensions()
    {
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    
    void gui_resource::draw()
    {
        if( texture -> gl_texture == 0x00 )                                     // Resource not yet loaded (just in case)
        {
            if( getDevMode() )
            {
                glBegin( GL_QUADS );                                            // Using rect.i.* as conversion shouldn't have happened yet
                {
                    ff::write( bqt_out, "Drawing temp graphic\n" );
                    
                    glColor4f( 0.0, 1.0, 1.0, 1.0 );                            // C
                    glVertex2f( 0, 0 );
                    
                    glColor4f( 1.0, 0.0, 1.0, 1.0 );                            // M
                    glVertex2f( dimensions[ 0 ], 0 );
                    
                    glColor4f( 1.0, 1.0, 0.0, 1.0 );                            // Y
                    glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                    
                    glColor4f( 0.0, 0.0, 0.0, 1.0 );                            // K
                    glVertex2f( 0, dimensions[ 1 ] );
                }
                glEnd();
            }
            
            return;
        }
        
        if( !dlist_created )
            createDisplayList();
        
        glCallList( gl_dlist );
    }
}


