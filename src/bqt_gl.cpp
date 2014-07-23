/* 
 * bqt_gl.cpp
 * 
 * Implements bqt_gl.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <GL/glew.h>
#include "bqt_gl.hpp"

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
}


