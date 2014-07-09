/* 
 * bqt_events.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_events.hpp"

#include <SDL2/SDL.h>
#include "bqt_taskexec.hpp"
#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool HandleEvents_task::execute( task_mask* caller_mask )
    {
        ff::write( bqt_out, "Task ", ( unsigned long )this, ": Handling events\n" );
        
        SDL_Delay( 500 );
        
        // bqt::submitTask( new HandleEvents_task() );
        
        return false;
    }
}


