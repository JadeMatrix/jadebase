/* 
 * jb_launchargs.cpp
 * 
 * Implements jb_launchargs.hpp & holds launch arguments state
 * Uses the getopt family http://linux.die.net/man/3/getopt
 * 
 * Note that no thread-safety is necessary, as launch flags/arguments are set up
 * at launch and do not change.
 * 
 * Also implements jb_log.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_launchargs.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>

#if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
#include <getopt.h>
#include <unistd.h>
#else
// TODO: Implement arg parsing on other platforms
#error "Launch argument parsing not implemented on non-POSIX platforms"
#endif

#include "jb_exception.hpp"
#include "jb_log.hpp"
#include "jb_settings.hpp"
#include "jb_version.hpp"

/* DEFAULTS *******************************************************************//******************************************************************************/

#define LAUNCHVAL_DEVMODE     false
#define LAUNCHVAL_LOGFILE     ""
#define LAUNCHVAL_TASKTHREADS 0
#define LAUNCHVAL_USETFILE    ""
#define LAUNCHVAL_SCRIPTFILE  ""

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // Options are immutable after parseLaunchArgs is called
    bool        dev_mode;
    std::string log_file_name;
    long        task_thread_limit;
    std::string user_settings_file;
    std::string main_script_file;
    
    std::filebuf log_fb;
    std::ostream log_stream( std::cout.rdbuf() );                               // Initialize to std::cout
    
    // Storage for launch arguments ////////////////////////////////////////////
    struct launcharg_data
    {
        jade::launcharg_callback callback;
        std::string long_flag;
        bool        require_arg;
        std::string arg_desc;
        std::string desc;
    };
    std::map< char, launcharg_data > parse_data;
    bool launch_args_parsed = false;
    
    std::string getFlagsString()                                                // Prints in simple ASCII order by flag for now
    {
        // Figure out padding widths ///////////////////////////////////////////
        
        int long_flag_width = 0;
        int  arg_desc_width = 0;
        
        for( std::map< char, launcharg_data >::iterator iter = parse_data.begin();
             iter != parse_data.end();
             ++iter )
        {
            if( long_flag_width < iter -> second.long_flag.length() )
                long_flag_width = iter -> second.long_flag.length();
            
            if( arg_desc_width < iter -> second.arg_desc.length() )
                arg_desc_width = iter -> second.arg_desc.length();
        }
        
        // Create whitespace string ////////////////////////////////////////////
        
        int whitespace_width = long_flag_width > arg_desc_width ? long_flag_width : arg_desc_width;
        char* whitespace = new char[ whitespace_width + 1 ];
        
        for( int i = 0; i < whitespace_width; ++i )
            whitespace[ i ] = ' ';
        whitespace[ whitespace_width ] = '\0';
        
        // Create formatted string /////////////////////////////////////////////
        
        std::string flags_string;
        
        for( std::map< char, launcharg_data >::iterator iter = parse_data.begin();
             iter != parse_data.end();
             ++iter )
        {
            ff::write( flags_string,
                       "[ -",
                       ff::ch( iter -> first ),
                       " | --",
                       iter -> second.long_flag,
                       whitespace + iter -> second.long_flag.length() + ( whitespace_width - long_flag_width ),
                       " ] ",
                       iter -> second.arg_desc,
                       whitespace + iter -> second.arg_desc.length() + ( whitespace_width - arg_desc_width ),
                       "       ",
                       iter -> second.desc,
                       "\n" );
        }
        
        ff::write( flags_string,
                   "Options are applied in order, so it is recommended to change the log file first to log any init errors" );
        
        // Return //////////////////////////////////////////////////////////////
        
        return flags_string;
    }
    
    // Launcharg parser functions //////////////////////////////////////////////
    
    bool parse_help( std::string arg )
    {
        ff::write( jb_out, getFlagsString(), "\n" );
        
        return false;
    }
    bool parse_version( std::string arg )
    {
        ff::write( jb_out, JADEBASE_VERSION_STRING, "\n" );
        
        return false;
    }
    bool parse_logFile( std::string arg )
    {
        log_fb.open( arg.c_str(), std::ios::out );
        
        if( !log_fb.is_open() )
        {
            jade::exception e;
            ff::write( *e,
                       "Could not open \"",
                       arg,
                       "\" to use as a log file" );
            throw e;
        }
        
        ff::write( jb_out, "Using file \"", arg, "\" for logging\n" );
        
        log_stream.rdbuf( &log_fb );
        
        return true;
    }
    bool parse_devMode( std::string arg )
    {
        ff::write( jb_out, "Developer mode enabled\n" );
        dev_mode = true;
        
        return true;
    }
    bool parse_taskThreads( std::string arg )
    {
        long arg_l = strtol( arg.c_str(), NULL, 0 );
        
        if( arg_l < 0 )
            throw jade::exception( "Task thread limit must be 0 or greater" );
        else
            task_thread_limit = arg_l;
        
        ff::write( jb_out, "Task threads limited to ", task_thread_limit, "\n" );
        
        return true;
    }
    bool parse_userConfig( std::string arg )
    {
        jade::loadSettingsFile( arg, true, true );
        user_settings_file = arg;
        
        return true;
    }
    bool parse_progConfig( std::string arg )
    {
        jade::loadSettingsFile( arg, false, true );
        
        return true;
    }
    bool parse_mainScript( std::string arg )
    {
        if( main_script_file.length() != 0 )
            throw jade::exception( "Only one script file may be loaded on startup" );
        
        main_script_file = arg;
        
        return true;
    }
}

/* jb_launchargs.hpp **********************************************************//******************************************************************************/

namespace jade
{
    void registerArgParser( launcharg_callback callback,
                            char        flag,
                            std::string long_flag,
                            bool        require_arg,
                            std::string arg_desc,
                            std::string desc )
    {
        if( launch_args_parsed )
            throw exception( "registerArgParser(): Launch arguments already parsed" );
        
        parse_data[ flag ].callback    = callback;
        parse_data[ flag ].long_flag   = long_flag;
        parse_data[ flag ].require_arg = require_arg;
        parse_data[ flag ].arg_desc    = arg_desc;
        parse_data[ flag ].desc        = desc;
    }
    
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    bool parseLaunchArgs( int argc, char* argv[] )
    {
        if( launch_args_parsed )
            throw exception( "parseLaunchArgs(): Launch arguments already parsed" );
        
        registerArgParser( parse_help,
                           'h',
                           "help",
                           false,
                           "",
                           "Prints this guide & exits" );
        registerArgParser( parse_version,
                           'v',
                           "version",
                           false,
                           "",
                           "Prints the software version & exits" );
        registerArgParser( parse_logFile,
                           'l',
                           "log-file",
                           true,
                           "FILE",
                           "Sets a log file, none by default" );
        registerArgParser( parse_devMode,
                           'd',
                           "dev-mode",
                           false,
                           "",
                           "Enables developer mode options" );
        registerArgParser( parse_taskThreads,
                           't',
                           "task-threads",
                           true,
                           "UINT",
                           "Limits the max number of task threads; 0 = no limit" );
        registerArgParser( parse_userConfig,
                           'g',
                           "user-config",
                           true,
                           "FILE",
                           "Load a user config file (saves changes to last file specified)" );
        registerArgParser( parse_progConfig,
                           'G',
                           "prog-config",
                           true,
                           "FILE",
                           "Load a program config file (does not save changes)" );
        registerArgParser( parse_mainScript,
                           's',
                           "script",
                           true,
                           "FILE",
                           "Load a Lua script" );
        
        launch_args_parsed = true;
        
        try
        {
            dev_mode           = LAUNCHVAL_DEVMODE;
            log_file_name      = LAUNCHVAL_LOGFILE;
            task_thread_limit  = LAUNCHVAL_TASKTHREADS;
            user_settings_file = LAUNCHVAL_USETFILE;
            main_script_file   = LAUNCHVAL_SCRIPTFILE;
            
            std::string val_str;
            
            struct option* long_flags = new struct option[ parse_data.size() + 1 ];
            int i = 0;
            for( std::map< char, launcharg_data >::iterator iter = parse_data.begin();
                 iter != parse_data.end();
                 ++iter )
            {
                val_str += iter -> first;
                if( iter -> second.require_arg )
                    val_str += ':';
                
                long_flags[ i ].name    = iter -> second.long_flag.c_str();
                long_flags[ i ].has_arg = iter -> second.require_arg ? required_argument : no_argument;
                long_flags[ i ].flag    = NULL;
                long_flags[ i ].val     = iter -> first;
                
                ++i;
            }
            long_flags[ i ].name    = NULL;
            long_flags[ i ].has_arg = 0x00;
            long_flags[ i ].flag    = NULL;
            long_flags[ i ].val     = '\0';
            
            int flag;
            bool return_val = true;
            
            while( return_val &&
                   ( flag = getopt_long( argc, argv, val_str.c_str(), long_flags, NULL ) ) != -1 )
            {
                if( !parse_data.count( flag ) )
                {
                    exception e;
                    ff::write( *e,
                               "Invalid flag specified; valid flags are:\n",
                               getFlagsString() );
                    delete[] long_flags;
                    throw e;
                }
                else
                    return_val = parse_data[ flag ].callback( optarg ? optarg : "" );   // optarg will be NULL if not used
            }
            
            delete[] long_flags;
            
            return return_val;
        }
        catch( exception e )
        {
            ff::write( jb_out, "parseLaunchArgs(): ", e.what(), "\n" );
            return false;
        }
    }
    #else
    #error Launch argument parsing not implemented on non-POSIX platforms
    #endif
    
    void initFromLaunchArgs()
    {
        // Nothing needed here yet
    }
    
    bool getDevMode()
    {
        return dev_mode;
    }
    std::string getLogFileName()
    {
        return log_file_name;
    }
    long getTaskThreadLimit()
    {
        return task_thread_limit;
    }
    std::string getUserSettingsFileName()
    {
        return user_settings_file;
    }
    std::string getMainScriptFileName()
    {
        return main_script_file;
    }
}

/* jb_log.hpp *****************************************************************//******************************************************************************/

namespace jade
{
    std::ostream& getLogStream()
    {
        return log_stream;
    }
    
    void closeLog()
    {
        // if( log_stream != NULL && log_stream != &std::cout )
        //     delete log_stream;
    }
}


