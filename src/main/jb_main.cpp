/* 
 * jb_main.cpp
 * 
 * Implements non-custom parts of jb_main.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_main.h"

#include "../utility/jb_exception.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

bool jb_parseLaunchArgs( int argc, char* argv[] )
{
    try
    {
        return jade::parseLaunchArgs( argc, argv );
    }
    catch( jade::exception& e )
    {
        ff::write( jb_out, e.what() );
    }
    
    return false;
}


