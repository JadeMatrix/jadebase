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
#include <vector>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool parseLaunchArgs( int argc, char* argv[] );                             // Returns true if the program should continue after parsing, false if exit
    void initFromLaunchArgs();                                                  // Does any special stuff we may need to do only after platform code is called
    
    bool          getDevMode();                                                 // Get whether developer mode is enabled
    std::string   getLogFileName();                                             // Get the path to the log file set at startup
    long          getTaskThreadLimit();                                         // Get the max number of threads available to the task system (>=1 or -1)
    const std::vector< std::string >& getStartupFiles();                        // Get vector of file names to open on startup
    std::string   getPreferenceFileName();                                      // Get the path to the preferences file set at startup
    
    // TODO: possibly allow -b flag for binary stream input
}

/******************************************************************************//******************************************************************************/

#endif


