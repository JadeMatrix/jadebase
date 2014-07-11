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

/* DEFAULTS *******************************************************************//******************************************************************************/

#define LAUNCHVAL_DEVMODE       false
#define LAUNCHVAL_LOGFILE       ""
#define LAUNCHVAL_TASKTHREADS   0
#define LAUNCHVAL_BLOCKEXPONENT 8
#define LAUNCHVAL_BLOCKEXP_RMAX 7
#define LAUNCHVAL_MAXUNDO       -1
#define LAUNCHVAL_PREFFILE      "~/.BQTDraw.cfg"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    bool parseLaunchArgs( int argc, char* argv[] );                             // Returns true if the program should continue after parsing, false if exit
    void initFromLaunchArgs();                                                  // Does any special stuff we may need to do only after platform code is called
    
    bool          getDevMode();                                                 // Get whether developer mode is enabled
    std::string   getLogFileName();                                             // Get the path to the log file set at startup
    long          getTaskThreadLimit();                                         // Get the max number of threads available to the task system (>=1 or -1)
    unsigned char getBlockExponent();                                           // Get the block size exponent: blocks are 2^exp x 2^exp textures
    long          getMaxUndoSteps();                                            // Get the max number of undos; -1 for unlimited
    const std::vector< std::string >* getStartupFiles();                        // Get vector of file names to open on startup
    std::string   getPreferenceFileName();                                      // Get the path to the preferences file set at startup
    
    // TODO: possibly allow -b flag for binary stream input
    
    // TODO: Move block exponent & undo steps to bqt_preferences.hpp
}

/******************************************************************************//******************************************************************************/

#endif


