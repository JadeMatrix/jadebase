/* 
 * bqt_main.cpp
 * 
 * Core program initialization
 * 
 * Pasting the FastFormat documentation here for now:
 * http://fastformat.sourceforge.net/documentation/index.html
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_main.h"

#include <iostream>

#include "bqt_launchargs.hpp"
#include "bqt_exception.hpp"
#include "bqt_threadutil.hpp"
#include "bqt_log.hpp"

/******************************************************************************//******************************************************************************/

bool bqt_parseLaunchArgs( int argc, char* argv[] )
{
    try
    {
        bqt::parseLaunchArgs( argc, argv );
    }
    catch( bqt::exception& e )
    {
        ff::write( bqt_out, e.what() );
        
        return false;
    }
    
    return true;
}

int bqt_main( /* int argc, char* argv[] */ )
{
    int exit_code = EXIT_FINE;
    
    try
    {
        bqt::initFromLaunchArgs();
        
        ff::write( bqt_out, "Hello World from BQTDraw!\n" );
        
        bqt_out << "testestest\n" << std::flush;
    }
    catch( bqt::exception& e )
    {
        // TODO: We want to log internal exceptions specially, maybe generate a
        // report or ticket of some kind?
        
        ff::write( bqt_out, e.what(), "\n" );
        
        exit_code = EXIT_BQTERR;
    }
    catch( std::exception& e )
    {
        ff::write( bqt_out, e.what(), "\n" );
        
        exit_code = EXIT_STDERR;
    }
    
    bqt::closeLog();
    
    return exit_code;
}


