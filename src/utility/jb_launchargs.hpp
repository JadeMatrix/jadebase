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
    typedef bool (* launcharg_callback )( std::string );                        // Argument parsing functions take a string and return a bool (whether the
                                                                                // parser should continue or should exit, for example after the version flag).
    
    void registerArgParser( launcharg_callback,                                 // The function to be called when the flag is encountered
                            char,                                               // The single-character version of the flag, e.g. "d" (parsed after a "-")
                            std::string,                                        // The long version of the flag, e.g. "developer-mode" (parsed after a "--")
                            bool,                                               // Whether the flag requires an argument to follow
                            std::string,                                        // A string describing what type of argument to pass, e.g. "FILE"
                            std::string );                                      // A help string describing the purpose and use of the flag
                                                                                // Note: registerArgParser() is not thread-safe
    
    bool parseLaunchArgs( int argc, char* argv[] );                             // Returns true if the program should continue after parsing, false if exit
    void initFromLaunchArgs();                                                  // Does any special stuff we may need to do only after platform code is called
    
    bool        getDevMode();                                                   // Get whether developer mode is enabled
    std::string getLogFileName();                                               // Get the path to the log file set at startup
    long        getTaskThreadLimit();                                           // Get the max number of threads available to the task system (>=1 or -1)
    std::string getUserSettingsFileName();                                      // Get the path to the user settings file set at startup
    std::string getMainScriptFileName();                                        // Get the path to the main Lua script file
    float       getGUIScaleOverride();                                          // Get the GUI scale override if any, or NaN if no override given
}

/******************************************************************************//******************************************************************************/

#endif


