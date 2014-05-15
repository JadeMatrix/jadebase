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

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <getopt.h>
#include <cstdlib>

#include "bqt_launchargs.hpp"
#include "bqt_log.hpp"

#include "bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct option long_flags[] = { { "devmode",           no_argument, NULL, 'd' },
                                   { "logfile",     required_argument, NULL, 'l' },
                                   { "taskthreads", required_argument, NULL, 't' },
                                   { 0,             0,                 0,    0   } };
    
    std::string flags_list = "[ -d | --devmode ]                Enables developer mode options\n"
                             "[ -l | --logfile ] FILE           Sets a log file, none by default\n"
                             "[ -t | --taskthreads ] NUMBER     Limits the max number of task threads; 0 = no limit; must be positive\n";
    
    // Engine options are immutable after parseLaunchArgs is called.
    bool        dev_mode;
    std::string log_file_name;
    long        task_thread_limit;
    
    std::ostream* log_stream;
}

/* bqt_launchargs.hpp *********************************************************//******************************************************************************/

namespace bqt
{
    void parseLaunchArgs( int argc, char* argv[] )
    {
        dev_mode          = LAUNCHVAL_DEVMODE;
        log_file_name     = LAUNCHVAL_LOGFILE;
        task_thread_limit = LAUNCHVAL_TASKTHREADS;
        
        log_stream = &std::cout;
        
        int flag;                                                               // <--
        
        while( ( flag = getopt_long( argc, argv, "dl:t:", long_flags, NULL ) ) != -1 )
        {
            switch( flag )
            {
            case 'd':
                ff::write( bqt_out, "Developer mode enabled\n" );
                dev_mode = true;
                break;
            case 'l':
                {
                    log_file_name = optarg;
                    if( log_stream != NULL && log_stream != &std::cout )        // Don't want to try to delete std::cout, that would be awkward
                        delete log_stream;
                    
                    ff::write( bqt_out, "Using file '", log_file_name, "' for logging\n" );
                    
                    std::ofstream* log_file = new std::ofstream( log_file_name.c_str() );
                    if( log_file == NULL || !( log_file -> is_open() ) )
                    {
                        throw exception( "Could not open \'"
                                              + log_file_name
                                              + "\' to use as a log file" );
                    }
                    
                    log_stream = log_file;
                }
                break;
            case 't':
                {
                    if( optarg < 0 )
                        throw exception( "Task thread limit must be 0 or greater" );
                    else
                        task_thread_limit = strtol( optarg, NULL, 10 );
                    
                    ff::write( bqt_out, "Task threads limited to ", task_thread_limit, "\n" );
                }
                break;
            default:
                throw exception( "Invalid flag specified; valid flags are:\n" + flags_list );
            }
        }
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
}

/* bqt_log.hpp ****************************************************************//******************************************************************************/

namespace bqt
{
    std::ostream* getLogStream()
    {
        return log_stream;
    }
    
    void closeLog()
    {
        if( log_stream != NULL && log_stream != &std::cout )
            delete log_stream;
    }
}


