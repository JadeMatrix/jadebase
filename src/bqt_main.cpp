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
#include "bqt_events.hpp"

// TODO: Debug, remove later?
#include "bqt_window.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class StartBQTDraw_task : public task
    {
    public:
        bool execute( task_mask* caller_mask )
        {
            const std::vector< std::string >* startup_files = getStartupFiles();
            
            {
                // submitTask( new HandleEvents_task() );
                
                submitTask( new window::manipulate( NULL ) );
                
                submitTask( new StopTaskSystem_task() );
            }
            
            ff::write( bqt_out, "Welcome to ", BQT_VERSION_STRING, "\n" );
            
            return true;
        }
        task_mask getMask()
        {
            return TASK_ALL;
        }
    };
}

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
            bqt::submitTask( new bqt::StartBQTDraw_task() );
            
            bqt::task_mask main_mask = bqt::TASK_TASK | bqt::TASK_SYSTEM;
            bqt::becomeTaskThread( &main_mask );
            
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


