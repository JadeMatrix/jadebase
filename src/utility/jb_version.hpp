#ifndef JADEBASE_VERSION_HPP
#define JADEBASE_VERSION_HPP

/* 
 * jb_version.hpp
 * 
 * Version information
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

/******************************************************************************//******************************************************************************/

namespace jade
{
    struct version
    {
        int major;
        int minor;
        int patch;
    };
    
    version&    getJadebaseVersion();
    std::string getJadebaseVersionString();                                     // Formatted version string
    
    void        setProgramName( std::string );
    std::string getProgramName();
    void        setProgramVersion( version& );
    void        setProgramVersion( int, int, int );                             // Major, minor, patch
    version     getProgramVersion();
    std::string getProgramVersionString();                                      // Formatted version string
}

/******************************************************************************//******************************************************************************/

#endif


