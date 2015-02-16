/* 
 * jb_main.cpp
 * 
 * Core program initialization
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_main.h"

#include <string>

#include <lua.hpp>
#include <pango/pangocairo.h>
#include <png.h>

#include "../gui/jb_named_resources.hpp"
#include "../tasking/jb_taskexec.hpp"
#include "../threading/jb_threadutil.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"
#include "../utility/jb_version.hpp"
#include "../windowsys/jb_events.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class StartJadebase_task : public task
    {
    public:
        bool execute( task_mask* caller_mask )
        {
            const std::vector< std::string >& startup_files = getStartupFiles();
            
            {
                // Your program startup code goes here
            }
            
            ff::write( jb_out,
                       "Welcome to ",
                       JADEBASE_VERSION_STRING,
                       ( jade::getDevMode() ? " (Developer Mode)" : "" ),
                       "\n" );
            
            ff::write( jb_out,
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

int jb_main()
{
    int exit_code = EXIT_FINE;
    
    try
    {
        jade::initFromLaunchArgs();
        
        if( jade::initTaskSystem( true ) )
        {
            jade::submitTask( new jade::StartJadebase_task() );
            
            jade::task_mask main_mask = jade::TASK_TASK | jade::TASK_SYSTEM;
            jade::becomeTaskThread( &main_mask );
            
            jade::deInitTaskSystem();
            
            std::string user_settings_file( jade::getUserSettingsFileName() );
            if( user_settings_file != "" )
                jade::saveSettings( user_settings_file );                       // Make sure settings are saved on exit; they should ideally be saved every time
                                                                                // they are changed.
        }
        else
            throw( jade::exception( "jb_main(): Failed to initialize task system" ) );
    }
    catch( jade::exception& e )
    {
        // TODO: We want to log internal exceptions specially, maybe generate a
        // report or ticket of some kind?
        
        ff::write( jb_out, "jadebase exception from main(): ", e.what(), "\n" );
        
        exit_code = EXIT_BQTERR;
    }
    catch( std::exception& e )
    {
        ff::write( jb_out, "Exception from main(): ", e.what(), "\n" );
        
        exit_code = EXIT_STDERR;
    }
    
    ff::write( jb_out, "Goodbye\n" );
    
    jade::closeLog();
    
    return exit_code;
}


