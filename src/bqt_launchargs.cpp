/* 
 * bqt_launchargs.cpp
 * 
 * Implements bqt_launchargs.hpp & holds launch arguments state
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include "bqt_launchargs.hpp"

#include "bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct option long_flags[] = { { "devmode",       no_argument, NULL, 'd' },
                                   { "logfile", required_argument, NULL, 'l' } };
    
    std::string flags_list = "[ -d | --devmode ]          Enables developer mode options\n"
                             "[ -l | --logfile ] FILE     Sets a log file, none by default\n";
    
    // Engine options are immutable after parseLaunchArgs is called.
    bool        dev_mode;
    std::string log_file_name;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void parseLaunchArgs( int argc, char* argv[] )
    {
        dev_mode      = LAUNCHVAL_DEVMODE;
        log_file_name = LAUNCHVAL_LOGFILE;
        
        while( int flag = getopt_long( argc, argv, "dl:", long_flags, NULL ) != -1 )
        {
            switch( flag )
            {
            case 'd':
                dev_mode = true;
                break;
            case 'l':
                log_file_name = optarg;
                break;
            default:
                throw bqt::exception( "Invalid flag specified; valid flags are:\n" + flags_list );
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
}


