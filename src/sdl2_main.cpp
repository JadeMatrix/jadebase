/* 
 * sdl2_bqt_main.cpp
 * 
 * bqt_main() wrapper for SDL2
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <SDL2/SDL.h>

#include "bqt_main.h"
#include "bqt_log.hpp"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )                                   // TODO: just what we need OR init everything + change POSIX stuff to SDL2
    {
        ff::write( bqt_out, "Could not initialize SDL2\n" );
        return -1;
    }
    else
    {
        int rval = 0;
        
        if( bqt_parseLaunchArgs( argc, argv ) )
        {
            rval = bqt_main();
            
            SDL_Quit();
        }
        
        return rval;
    }
}


