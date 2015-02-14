/* 
 * sdl2_jb_main.cpp
 * 
 * jb_main() wrapper for SDL2
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <SDL2/SDL.h>

#include "jb_main.h"
#include "jb_log.hpp"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )                                   // TODO: just what we need OR init everything + change POSIX stuff to SDL2
    {
        ff::write( jb_out, "Could not initialize SDL2\n" );
        return -1;
    }
    else
    {
        int rval = 0;
        
        if( jb_parseLaunchArgs( argc, argv ) )
        {
            rval = jb_main();
            
            SDL_Quit();
        }
        
        return rval;
    }
}


