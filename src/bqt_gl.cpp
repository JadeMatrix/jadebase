/* 
 * bqt_gl.cpp
 * 
 * Implements bqt_gl.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gl.hpp"

#include <string>

#include "bqt_task.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_log.hpp"

#include "bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    class InitOpenGL_task : public bqt::task
    {
        bool execute( bqt::task_mask* caller_mask )
        {
            // ff::write( bqt_out, "Adding TASK_GPU to a task thread mask\n" );
            *caller_mask |= bqt::TASK_GPU;
            return true;
        }
        bqt::task_mask getMask()
        {
            return bqt::TASK_ALL;
        }
        bqt::task_priority getPriority()
        {
            return bqt::PRIORITY_HIGH;
        }
    };
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void initOpenGL()
    {
        submitTask( new InitOpenGL_task() );
    }
    
    GLuint bytesToTexture( unsigned char* data,
                           unsigned int w,
                           unsigned int h,
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
                      0,
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
        
        glBindTexture( GL_TEXTURE_2D, 0x00 );
        
        if( gl_error != GL_NO_ERROR )
        {
            bqt::exception e;
            ff::write( *e,
                       "bytesToTexture(): OpenGL error 0x",
                       ff::to_x( ( unsigned long )gl_error ),
                       " (",
                       ( const char* )glewGetErrorString( gl_error ),
                       ") loading pixels from 0x",
                       ff::to_x( ( unsigned long )( data ), HEX_WIDTH, HEX_WIDTH ),
                       " to texture 0x",
                       ff::to_x( original, HEX_WIDTH, HEX_WIDTH ) );
            throw e;
        }
        
        return original;
    }
    
    void addDrawMask( int x, int y, unsigned int w, unsigned int h )
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


