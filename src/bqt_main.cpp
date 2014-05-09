/* 
 * bqt_main.cpp
 * 
 * Core program initialization
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>

#include "bqt_launchargs.hpp"
#include "bqt_threadutil.hpp"

#include "bqt_mutex.hpp"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    bqt::mutex testm;
    
    int exit_code = EXIT_FINE;
    
    try
    {
        bqt::parseLaunchArgs( argc, argv );
    }
    catch( std::exception& e )
    {
        std::cout << e.what();
        
        exit_code = EXIT_STDERR;
    }
    
    return exit_code;
}


