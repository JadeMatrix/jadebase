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
#include <unistd.h>
#include <getopt.h>
#include <cstdlib>
#include <cmath>

#include "bqt_log.hpp"
#include "bqt_exception.hpp"
#include "bqt_imagemode.hpp"
#include "bqt_version.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct option long_flags[] = { {        "help",       no_argument, NULL, 'h' },
                                   {     "version",       no_argument, NULL, 'v' },
                                   {     "devmode",       no_argument, NULL, 'd' },
                                   {     "logfile", required_argument, NULL, 'l' },
                                   { "taskthreads", required_argument, NULL, 't' },
                                   {    "blockexp", required_argument, NULL, 'e' },
                                   {             0,                 0,    0,   0 } };
    
    std::string flags_list = "[ -h | --help        ]            Prints this guide & exits\n"
                             "[ -v | --version     ]            Prints the software version & exits\n"
                             "[ -d | --devmode     ]            Enables developer mode options\n"
                             "[ -l | --logfile     ] FILE       Sets a log file, none by default\n"
                             "[ -t | --taskthreads ] UINT       Limits the max number of task threads; 0 = no limit\n"
                             "[ -e | --blockexp    ] UINT       Sets the block texture size: 2^exp x 2^exp; 1 <= exp <= 255\n";
    
    // Engine options are immutable after parseLaunchArgs is called.
    bool          dev_mode;
    std::string   log_file_name;
    long          task_thread_limit;
    unsigned char block_exponent;
    
    std::filebuf log_fb;
    std::ostream log_stream( std::cout.rdbuf() );                               // Initialize to std::cout
}

/* bqt_launchargs.hpp *********************************************************//******************************************************************************/

namespace bqt
{
    bool parseLaunchArgs( int argc, char* argv[] )
    {
        dev_mode          = LAUNCHVAL_DEVMODE;
        log_file_name     = LAUNCHVAL_LOGFILE;
        task_thread_limit = LAUNCHVAL_TASKTHREADS;
        block_exponent    = LAUNCHVAL_BLOCKEXPONENT;
        
        int flag;                                                               // <--
        
        while( ( flag = getopt_long( argc, argv, "hvdl:t:", long_flags, NULL ) ) != -1 )
        {
            switch( flag )
            {
            case 'h':
                ff::write( bqt_out, flags_list );
                return false;
            case 'v':
                ff::write( bqt_out, BQT_VERSION_STRING, "\n" );
                return false;
            case 'd':
                ff::write( bqt_out, "Developer mode enabled\n" );
                dev_mode = true;
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
            case 'e':
                {
                    long optarg_l = strtol( optarg, NULL, 0 );
                    
                    if( optarg_l < BLOCKEXPONENT_MIN || optarg_l > BLOCKEXPONENT_MAX )
                        throw exception( "Block size exponent must be between " MACROTOSTR( BLOCKEXPONENT_MIN ) " and " MACROTOSTR( BLOCKEXPONENT_MAX ) );
                    else
                    {
                        block_exponent = optarg_l;
                        
                        if( block_exponent < LAUNCHVAL_BLOCKEXP_RMAX )
                            ff::write( bqt_out, "Warning: block exponent below recommended minimum (", LAUNCHVAL_BLOCKEXP_RMAX, ")\n" );
                    }
                    
                    long block_w = pow( 2, block_exponent );
                    ff::write( bqt_out, "Block size set to ", block_w, " x ", block_w, "\n" );
                }
                break;
            default:
                throw exception( "Invalid flag specified; valid flags are:\n" + flags_list );
            }
        }
        
        return true;
    }
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
    unsigned char getBlockExponent()
    {
        return block_exponent;
    }
}

/* bqt_log.hpp ****************************************************************//******************************************************************************/

namespace bqt
{
    std::ostream* getLogStream()
    {
        return &log_stream;
    }
    
    void closeLog()
    {
        // if( log_stream != NULL && log_stream != &std::cout )
        //     delete log_stream;
    }
}


