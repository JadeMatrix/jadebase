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

/******************************************************************************//******************************************************************************/

class debugPrintClass : public bqt::task
{
protected:
    std::string str;
public:
    debugPrintClass( std::string str )
    {
        this -> str = str;
    }
    bool execute( bqt::task_mask* caller_mask )
    {
        ff::write( bqt_out, "\"", str, "\" from #", ( unsigned int )( *caller_mask ), "\n" );
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

int bqt_main( /* int argc, char* argv[] */ )
{
    int exit_code = EXIT_FINE;
    
    try
    {
        bqt::initFromLaunchArgs();
        
        if( bqt::initTaskSystem( true ) )
        {
            for( int i = 0; i < 20; i++ )
                bqt::submitTask( new debugPrintClass( "Hello World" ) );
            
            bqt::submitTask( new bqt::StopTaskSystem_task() );
            
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


