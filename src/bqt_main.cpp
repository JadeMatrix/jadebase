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

#include <lua.hpp>
#include <pango/pangocairo.h>
#include <png.h>

#include "bqt_launchargs.hpp"
#include "bqt_exception.hpp"
#include "threading/bqt_threadutil.hpp"
#include "bqt_log.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_version.hpp"
#include "bqt_events.hpp"
#include "bqt_gl.hpp"
#include "gui/bqt_named_resources.hpp"

// TODO: Debug, remove later
#include "bqt_window.hpp"
#include <cstdlib>
#include <unistd.h>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class StartBQTDraw_task : public task
    {
    public:
        bool execute( task_mask* caller_mask )
        {
            const std::vector< std::string >& startup_files = getStartupFiles();
            
            {
                submitTask( new HandleEvents_task() );
                
                submitTask( new window::manipulate( NULL ) );
                
                
            }
            
            ff::write( bqt_out,
                       "Welcome to ",
                       BQT_VERSION_STRING,
                       ( bqt::getDevMode() ? " (Developer Mode)" : "" ),
                       "\n" );
            
            ff::write( bqt_out,
                       "Using:\n",
                       "  - Cairo ", cairo_version_string(), " [ http://cairographics.org ]\n",
                       "  - libpng ", PNG_LIBPNG_VER / 10000,
                                      ".",
                                      ( PNG_LIBPNG_VER / 100 ) % 100,
                                      ".",
                                      PNG_LIBPNG_VER % 100, " [ http://libpng.org ] \n",
                       "  - ", LUA_VERSION, " [ http://lua.org ]\n",
                       "  - Pango ", pango_version_string(), " [ http://pango.org ]\n" );
            
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
        
        ff::write( bqt_out, "BQTDraw exception from main(): ", e.what(), "\n" );
        
        exit_code = EXIT_BQTERR;
    }
    catch( std::exception& e )
    {
        ff::write( bqt_out, "Exception from main(): ", e.what(), "\n" );
        
        exit_code = EXIT_STDERR;
    }
    
    bqt::closeLog();
    
    return exit_code;
}


