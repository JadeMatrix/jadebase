#ifndef BQT_LAUNCHARGS_HPP
#define BQT_LAUNCHARGS_HPP

/* 
 * bqt_launchargs.hpp
 * 
 * Functions for parsing & accessing launch arguments and their values
 * 
 * None of the getXYZ() functions are considered valid until parseLaunchArgs()
 * has been called, after which they will always return the same value.  It is
 * assumed the programe will only call parseLaunchArgs() once, on startup.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

/* DEFAULTS *******************************************************************//******************************************************************************/

// TODO: Ensure all launch flags supported are documented in launch.sh

// #define LAUNCHARG_DEVMODE       "--devmode"
// #define LAUNCHFLG_DEVMODE       "-d"
#define LAUNCHVAL_DEVMODE       false

// #define LAUNCHARG_LOGFILE       "--logfile"
// #define LAUNCHFLG_LOGFILE       "-l"
#define LAUNCHVAL_LOGFILE       ""

// #define LAUNCHARG_TASKTHREADS   "--taskthreads"
// #define LAUNCHFLG_TASKTHREADS   "-t"
#define LAUNCHVAL_TASKTHREADS   0

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void parseLaunchArgs( int argc, char* argv[] );
    void initFromLaunchArgs();                                                  // Does any special stuff we may need to do only after platform code is called
    
    bool        getDevMode();                                                   // Get whether developer mode is enabled
    std::string getLogFileName();                                               // Get the path to the log file set at startup
    long        getTaskThreadLimit();                                           // Get the max number of threads available to the task system (>=1 or -1)
}

/******************************************************************************//******************************************************************************/

#endif


