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

#include <string>

#include "bqt_launchargs.hpp"
#include "bqt_exception.hpp"
#include "bqt_threadutil.hpp"
#include "bqt_log.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_version.hpp"
#include "bqt_window.hpp"

/******************************************************************************//******************************************************************************/

class StartBQTDraw_task : public bqt::task
{
public:
    bool execute( bqt::task_mask* caller_mask )
    {
        const std::vector< std::string >* startup_files = bqt::getStartupFiles();
        
        {
            bqt::submitTask( new bqt::window::manipulate( NULL ) );
            
            ff::write( bqt_out, "Welcome to ", BQT_VERSION_STRING, "\n" );
            
            bqt::submitTask( new bqt::StopTaskSystem_task() );
        }
        
        return true;
    }
    bqt::task_mask getMask()
    {
        return bqt::TASK_ALL;
    }
};

/******************************************************************************//******************************************************************************/

bool bqt_parseLaunchArgs( int argc, char* argv[] )
{
    try
    {
        return bqt::parseLaunchArgs( argc, argv );
    }
    catch( bqt::exception& e )
    {
        ff::write( bqt_out, e.what() );
    }
    
    return false;
}

int bqt_main()
{
    int exit_code = EXIT_FINE;
    
    try
    {
        bqt::initFromLaunchArgs();
        
        if( bqt::initTaskSystem( true ) )
        {
            bqt::submitTask( new StartBQTDraw_task() );
            
            bqt::task_mask main_mask = bqt::TASK_TASK | bqt::TASK_SYSTEM;
            bqt::becomeTaskThread( &main_mask );
            
            // bqt::window::manipulate test_m = bqt::window::manipulate( NULL );
            // test_m.execute( &main_mask );
            
            // // SDL_Window* test_w = SDL_CreateWindow( BQT_WINDOW_DEFAULT_NAME,
            // //                            SDL_WINDOWPOS_CENTERED,
            // //                            SDL_WINDOWPOS_CENTERED,
            // //                            BQT_WINDOW_DEFAULT_WIDTH,
            // //                            BQT_WINDOW_DEFAULT_HEIGHT,
            // //                            SDL_WINDOW_OPENGL );
            
            // SDL_Delay( 5000 );
            
            // SDL_DestroyWindow( test_w );
            
            bqt::deInitTaskSystem();
        }
        else
            throw( bqt::exception( "bqt_main(): Failed to initialize task system" ) );
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


