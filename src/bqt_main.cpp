/* 
 * bqt_main.cpp
 * 
 * Core program initialization
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>

#include <fastformat/ff.hpp>
#include <fastformat/sinks/ostream.hpp>

#include "bqt_launchargs.hpp"
#include "bqt_exception.hpp"
#include "bqt_threadutil.hpp"
#include "bqt_log.hpp"

/******************************************************************************//******************************************************************************/

int main( int argc, char* argv[] )
{
    int exit_code = EXIT_FINE;
    
    try
    {
        bqt::parseLaunchArgs( argc, argv );
        
        std::cout << "OStream testing\n";
        ff::fmt( bqt_out, "FastFormat testing{0}", "\n" );
    }
    catch( bqt::exception& e )
    {
        // TODO: We want to log internal exceptions specially, maybe generate a
        // report or ticket of some kind?
        
        ff::write( std::cerr, e.what() );
        ff::write( bqt_out, e.what() );                                         // Won't write if bqt_out is invalid, so we write to std::cerr first
                                                                                // This actually ends up being pretty nice, as it gets logged in both places
        
        exit_code = EXIT_BQTERR;
    }
    catch( std::exception& e )
    {
        ff::write( std::cerr, e.what() );
        ff::write( bqt_out, e.what() );
        
        exit_code = EXIT_STDERR;
    }
    
    bqt::closeLog();
    
    return exit_code;
}


