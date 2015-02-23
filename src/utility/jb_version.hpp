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
    std::string getJadebaseVersionString();
    
    void        setProgramName( std::string n );
    std::string getProgramName();
    void        setProgramVersion( version& v );
    void        setProgramVersion( int major, int minor, int patch );
    version     getProgramVersion();
    std::string getProgramVersionString();
}

/******************************************************************************//******************************************************************************/

#endif


