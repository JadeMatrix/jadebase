#ifndef JADEBASE_LAUNCHARGS_HPP
#define JADEBASE_LAUNCHARGS_HPP

/* 
 * jb_launchargs.hpp
 * 
 * Functions for parsing & accessing launch arguments and their values
 * 
 * None of the getXYZ() functions are considered valid until parseLaunchArgs()
 * has been called, after which they will always return the same value.  It is
 * assumed the program will only call parseLaunchArgs() once, on startup.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>
#include <vector>

/******************************************************************************//******************************************************************************/

namespace jade
{
    typedef bool (* launcharg_callback )( std::string arg );
    void registerArgParser( launcharg_callback callback,
                            char        flag,
                            std::string long_flag,
                            bool        require_arg,
                            std::string arg_desc,
                            std::string desc );                                 // Not thread-safe
    
    bool parseLaunchArgs( int argc, char* argv[] );                             // Returns true if the program should continue after parsing, false if exit
    void initFromLaunchArgs();                                                  // Does any special stuff we may need to do only after platform code is called
    
    bool          getDevMode();                                                 // Get whether developer mode is enabled
    std::string   getLogFileName();                                             // Get the path to the log file set at startup
    long          getTaskThreadLimit();                                         // Get the max number of threads available to the task system (>=1 or -1)
    std::string   getUserSettingsFileName();                                    // Get the path to the user settings file set at startup
}

/******************************************************************************//******************************************************************************/

#endif


