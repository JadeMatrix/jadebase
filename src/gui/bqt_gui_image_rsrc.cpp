/* 
 * bqt_gui_image_rsrc.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_image_rsrc.hpp"

#include "../bqt_launchargs.hpp"
#include "bqt_named_resources.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void image_rsrc::createDisplayList()
    {
        double d_position  [ 2 ];
        double d_dimensions[ 2 ];
        
        d_position  [ 0 ] = ( double )tex_pos   [ 0 ] / ( double )( texture -> dimensions[ 1 ] );
        d_position  [ 1 ] = ( double )tex_pos   [ 1 ] / ( double )( texture -> dimensions[ 0 ] );
        d_dimensions[ 0 ] = ( double )dimensions[ 0 ] / ( double )( texture -> dimensions[ 1 ] );
        d_dimensions[ 1 ] = ( double )dimensions[ 1 ] / ( double )( texture -> dimensions[ 0 ] );
        
        gl_dlist = glGenLists( 1 );
        
        glNewList( gl_dlist, GL_COMPILE );
        {
            glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
            
            glBegin( GL_QUADS );
            {
                glTexCoord2d( d_position[ 0 ], d_position[ 1 ] );
                glVertex2f( 0, 0 );
                
                glTexCoord2d( d_position[ 0 ], d_position[ 1 ] + d_dimensions[ 1 ] );
                glVertex2f( 0, dimensions[ 1 ] );
                
                glTexCoord2d( d_position[ 0 ] + d_dimensions[ 0 ], d_position[ 1 ] + d_dimensions[ 1 ] );
                glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                
                glTexCoord2d( d_position[ 0 ] + d_dimensions[ 0 ], d_position[ 1 ] );
                glVertex2f( dimensions[ 0 ], 0 );
            }
            glEnd();
            
            glBindTexture( GL_TEXTURE_2D, 0x00 );                               // Unbind texture
        }
        glEndList();
        
        dlist_created = true;
    }
    
    void image_rsrc::drawTempGraphic()
    {
        glBegin( GL_QUADS );                                                    // Using rect.i.* as conversion shouldn't have happened yet
        {
            glColor4f( 0.0, 1.0, 1.0, 1.0 );                                    // C
            glVertex2f( 0, 0 );
            
            glColor4f( 1.0, 0.0, 1.0, 1.0 );                                    // M
            glVertex2f( dimensions[ 0 ], 0 );
            
            glColor4f( 1.0, 1.0, 0.0, 1.0 );                                    // Y
            glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
            
            glColor4f( 0.0, 0.0, 0.0, 1.0 );                                    // K
            glVertex2f( 0, dimensions[ 1 ] );
        }
        glEnd();
    }
    
    image_rsrc::image_rsrc( std::string f,
                            unsigned int x,
                            unsigned int y,
                            unsigned int w,
                            unsigned int h ) : gui_resource( w, h )
    {
        texture = acquireTexture( f );
        
        dlist_created = false;
        
        tex_pos[ 0 ] = x;
        tex_pos[ 1 ] = y;
    }
    image_rsrc::~image_rsrc()
    {
        if( dlist_created )
            glDeleteLists( gl_dlist, 1 );
        
        releaseTexture( texture );
    }
    
    void image_rsrc::draw()
    {
        if( texture -> gl_texture == 0x00 )
            // && getDevMode() )                                                   // Resource not yet loaded (just in case)
        {
            drawTempGraphic();
            return;
        }
        
        if( !dlist_created )
            createDisplayList();
        
        glCallList( gl_dlist );
    }
}


