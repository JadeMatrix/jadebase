/* 
 * jb_gl.cpp
 * 
 * Implements jb_gl.hpp
 * 
 */

// http://www.opengl.org/wiki/Texture_Storage#Texture_copy

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_gl.hpp"

#include <string>

#include "jb_exception.hpp"
#include "jb_log.hpp"
#include "../tasking/jb_task.hpp"
#include "../tasking/jb_taskexec.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    class InitOpenGL_task : public jade::task
    {
        bool execute( jade::task_mask* caller_mask )
        {
            // ff::write( jb_out, "Adding TASK_GPU to a task thread mask\n" );
            *caller_mask |= jade::TASK_GPU;
            return true;
        }
        jade::task_mask getMask()
        {
            return jade::TASK_ALL;
        }
        jade::task_priority getPriority()
        {
            return jade::PRIORITY_HIGH;
        }
    };
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    void initOpenGL()
    {
        submitTask( new InitOpenGL_task() );
    }
    
    GLuint bytesToTexture( unsigned char* data,
                           dpi::pixels w,
                           dpi::pixels h,
                           unsigned int level,
                           GLuint original )
    {
        if( data == NULL )
            throw exception( "bytesToTexture(): Data is NULL" );
        
        if( original == 0x00 )
            glGenTextures( 1, &original );
        
        if( original == 0x00 )
            throw exception( "bytesToTexture(): Could not generate texture" );
        
        glBindTexture( GL_TEXTURE_2D, original );
        glTexImage2D( GL_TEXTURE_2D,
                      level,
                      GL_RGBA,
                      w,
                      h,
                      0,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
        GLenum gl_error = glGetError();
        
        glBindTexture( GL_TEXTURE_2D, 0x00 );                                   // Un-bind texture
        
        if( gl_error != GL_NO_ERROR )
        {
            jade::exception e;
            ff::write( *e,
                       "bytesToTexture(): OpenGL error 0x",
                       ff::to_x( ( unsigned long )gl_error ),
                       " (",
                       ( const char* )glewGetErrorString( gl_error ),
                       ") loading pixels from 0x",
                       ff::to_x( ( unsigned long )( data ), PTR_HEX_WIDTH, PTR_HEX_WIDTH ),
                       " to texture 0x",
                       ff::to_x( original, PTR_HEX_WIDTH, PTR_HEX_WIDTH ) );
            throw e;
        }
        
        return original;
    }
    
    void addDrawMask( dpi::points x,
                      dpi::points y,
                      dpi::points w,
                      dpi::points h )
    {
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
        
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
        
        glBegin( GL_QUADS );
        {
            glVertex2f( x    , y     );
            glVertex2f( x    , y + h );
            glVertex2f( x + w, y + h );
            glVertex2f( x + w, y     );
        }
        glEnd();
        
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    }
    void clearDrawMasks()
    {
        glClear( GL_STENCIL_BUFFER_BIT );
        glStencilFunc( GL_ALWAYS, 1, 1 );
        // glDisable( GL_STENCIL_TEST );
    }
}


