/* 
 * jb_test.cpp
 * 
 * Simple jadebase test program
 * 
 * make/build/jb_test -d -G "resources/cfg/defaults_general.cfg" -G "resources/cfg/defaults_linux.cfg" -s "src/jb_test.lua" -g "local/test.cfg"
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include <jadebase/jadebase.hpp>
#include <lua.hpp>
#include <pango/pangocairo.h>
#include <png.h>

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    std::vector< std::string > startup_files;
    
    bool parse_StartupFiles( std::string arg )
    {
        startup_files.push_back( arg );
        
        return true;
    }
    
    const std::vector< std::string >& getStartupFiles()                         // Get vector of file names to open on startup
    {
        return startup_files;
    }
}

namespace jade
{
    class StartJadebase_task : public task
    {
    public:
        bool execute( task_mask* caller_mask )
        {
            setProgramName( "jb_test" );
            setProgramVersion( 1, 0, 0 );
            
            ff::write( jb_out,
                       "Welcome to ",
                       getProgramVersionString(),
                       ( getDevMode() ? " (Developer Mode)" : "" ),
                       "\n" );
            
            ff::write( jb_out,
                       "Using:\n",
                       "  - ", getJadebaseVersionString(), " [ http://github.com/JadeMatrix/jadebase/ ]\n",
                       "  - Cairo ", cairo_version_string(), " [ http://cairographics.org/ ]\n",
                       "  - libpng ", PNG_LIBPNG_VER / 10000,
                                      ".",
                                      ( PNG_LIBPNG_VER / 100 ) % 100,
                                      ".",
                                      PNG_LIBPNG_VER % 100, " [ http://libpng.org/ ]\n",
                       "  - ", LUA_VERSION, " [ http://lua.org ]\n",
                       "  - Pango ", pango_version_string(), " [ http://pango.org/ ]\n" );
            
            { // Your program startup code goes here ///////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                // window* test_window = new window();
                
                // window::manipulate* manip = new window::manipulate( test_window );
                
                // manip -> setTitle( getProgramVersionString() + " (C++)" );
                
                // submitTask( new window::manipulate( test_window ) );
                
                // {
                //     std::shared_ptr< group > group_sp = std::make_shared< group >( 0, 0, 256, 256 );
                    
                //     std::shared_ptr< button > butt_sp = std::make_shared< button >( 10, 10, 150, 27 );
                //     group_sp -> addElement( std::dynamic_pointer_cast< gui_element >( butt_sp ) );
                    
                //     std::shared_ptr< dial > dial_sp = std::make_shared< dial >( 180, 10 );
                //     group_sp -> addElement( std::dynamic_pointer_cast< gui_element >( dial_sp ) );
                    
                //     std::shared_ptr< tabset::tab > tab_sp = std::make_shared< tabset::tab >( nullptr, "Test Tab", group_sp );
                    
                //     std::shared_ptr< tabset > ts_sp = std::make_shared< tabset >( 0, 0, 256, 256 );
                //     ts_sp -> addTab( tab_sp );
                    
                //     test_window -> getTopElement() -> addElement(std::dynamic_pointer_cast< gui_element >( ts_sp )  );
                // }
                
                // test_window -> requestRedraw();
                
            } //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            std::string main_script_file = getMainScriptFileName();
            if( main_script_file.length() != 0 )
            {
                lua_state& global_lua_state( getGlobalLuaState() );
                global_lua_state.open( main_script_file );
                global_lua_state.run();
            }
            
            return true;
        }
        task_mask getMask()
        {
            return TASK_ALL;
        }
    };
}

/******************************************************************************//******************************************************************************/

void jb_registerLaunchArgs()
{
    jade::registerArgParser( parse_StartupFiles,
                             'f',
                             "open-file",
                             true,
                             "FILE",
                             "Opens file on startup" );
}

int jb_main()
{
    // TODO: Move most of this to main()
    
    int exit_code = EXIT_FINE;
    
    try
    {
        jade::initFromLaunchArgs();
        
        // if( jade::initTaskSystem( ( long )0 ) )
        if( jade::initTaskSystem( true ) )
        {
            jade::initNamedResources();
            jade::initGlobalLuaState();
            
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
        
        exit_code = EXIT_JBERR;
    }
    catch( std::exception& e )
    {
        ff::write( jb_out, "Exception from main(): ", e.what(), "\n" );
        
        exit_code = EXIT_STDERR;
    }
    
    jade::deinitGlobalLuaState();
    
    ff::write( jb_out, "Goodbye\n" );
    
    jade::closeLog();
    
    return exit_code;
}


