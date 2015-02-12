/* 
 * bqt_launchargs.cpp
 * 
 * Implements bqt_launchargs.hpp & holds launch arguments state
 * Using the getopt family http://linux.die.net/man/3/getopt
 * 
 * Note that no thread-safety is necessary as launch flags/arguments are set up
 * at launch and do not change.
 * 
 * Also implements bqt_log.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_launchargs.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
#include <unistd.h>
#include <getopt.h>
#else
// TODO: Implement arg parsing on other platforms
#error "Launch argument parsing not implemented on non-POSIX platforms"
#endif

#include "bqt_log.hpp"
#include "bqt_exception.hpp"
#include "bqt_version.hpp"
#include "bqt_settings.hpp"

/* DEFAULTS *******************************************************************//******************************************************************************/

#define LAUNCHVAL_DEVMODE       false
#define LAUNCHVAL_LOGFILE       ""
#define LAUNCHVAL_TASKTHREADS   0
#define LAUNCHVAL_USETFILE      ""

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    struct option long_flags[] = { {         "help",       no_argument, NULL, 'h' },
                                   {      "version",       no_argument, NULL, 'v' },
                                   {    "open-file", required_argument, NULL, 'f' },
                                   {     "log-file", required_argument, NULL, 'l' },
                                   {     "dev-mode",       no_argument, NULL, 'd' },
                                   { "task-threads", required_argument, NULL, 't' },
                                   {  "user-config", required_argument, NULL, 'g' },
                                   {  "prog-config", required_argument, NULL, 'G' },
                                   {              0,                 0,    0,   0 } };
    #else
    #error "Launch argument parsing not implemented on non-POSIX platforms"
    #endif
    
    std::string flags_list = "[ -h | --help         ]            Prints this guide & exits\n"
                             "[ -v | --version      ]            Prints the software version & exits\n"
                             "[ -f | --open-file    ] FILE       Opens file on startup\n"
                             "[ -l | --log-file     ] FILE       Sets a log file, none by default\n"
                             "[ -d | --dev-mode     ]            Enables developer mode options\n"
                             "[ -t | --task-threads ] UINT       Limits the max number of task threads; 0 = no limit\n"
                             "[ -g | --user-config  ] FILE       Load a user config file (saves changes to last file specified)\n"
                             "[ -G | --prog-config  ] FILE       Load a program config file (does not save changes)\n"
                             "Options are applied in order, so it is recommended to change the log file first to log any init errors\n";
    
    // Engine options are immutable after parseLaunchArgs is called.
    bool          dev_mode;
    std::string   log_file_name;
    long          task_thread_limit;
    std::vector< std::string > startup_files;
    std::string   user_settings_file;
    
    std::filebuf log_fb;
    std::ostream log_stream( std::cout.rdbuf() );                               // Initialize to std::cout
}

/* bqt_launchargs.hpp *********************************************************//******************************************************************************/

namespace bqt
{
    #if defined PLATFORM_XWS_GNUPOSIX | defined PLATFORM_MACOSX
    bool parseLaunchArgs( int argc, char* argv[] )
    {
        try
        {
            dev_mode           = LAUNCHVAL_DEVMODE;
            log_file_name      = LAUNCHVAL_LOGFILE;
            task_thread_limit  = LAUNCHVAL_TASKTHREADS;
            user_settings_file = LAUNCHVAL_USETFILE;
            
            int flag;                                                           // <--
            
            while( ( flag = getopt_long( argc, argv, "hvf:l:dt:g:G:", long_flags, NULL ) ) != -1 )
            {
                switch( flag )
                {
                case 'h':
                    ff::write( bqt_out, flags_list );
                    return false;
                case 'v':
                    ff::write( bqt_out, BQT_VERSION_STRING, "\n" );
                    return false;
                case 'f':
                    {
                        ff::write( bqt_out, "File opening not implemented yet\n" ); // TODO: implement
                    }
                    break;
                case 'l':
                    {
                        log_file_name = optarg;
                        
                        log_fb.open( log_file_name.c_str(), std::ios::out );
                        
                        if( !log_fb.is_open() )
                        {
                            throw exception( "Could not open \'"
                                             + log_file_name
                                             + "\' to use as a log file" );
                        }
                        
                        ff::write( bqt_out, "Using file '", log_file_name, "' for logging\n" );
                        
                        log_stream.rdbuf( &log_fb );
                    }
                    break;
                case 'd':
                    ff::write( bqt_out, "Developer mode enabled\n" );
                    dev_mode = true;
                    break;
                case 't':
                    {
                        long optarg_l = strtol( optarg, NULL, 0 );
                        
                        if( optarg_l < 0 )
                            throw exception( "Task thread limit must be 0 or greater" );
                        else
                            task_thread_limit = optarg_l;
                        
                        ff::write( bqt_out, "Task threads limited to ", task_thread_limit, "\n" );
                    }
                    break;
                case 'g':
                    {
                        loadSettingsFile( optarg, true, true );
                        user_settings_file = optarg;
                    }
                    break;
                case 'G':
                    {
                        loadSettingsFile( optarg, false, true );
                    }
                    break;
                default:
                    throw exception( "Invalid flag specified; valid flags are:\n" + flags_list );
                }
            }
            
            return true;
        }
        catch( exception e )
        {
            ff::write( bqt_out, "parseLaunchArgs(): ", e.what(), "\n" );
            return false;
        }
    }
    #else
    #error "Launch argument parsing not implemented on non-POSIX platforms"
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
    const std::vector< std::string >& getStartupFiles()
    {
        return startup_files;
    }
    std::string getUserSettingsFileName()
    {
        return user_settings_file;
    }
}

/* bqt_log.hpp ****************************************************************//******************************************************************************/

namespace bqt
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


