/* 
 * jb_version.cpp
 * 
 * Implements jb_version.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_version.hpp"

#include "jb_log.hpp"
#include "../threading/jb_mutex.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

#define JADEBASE_VERSION_MAJOR   0
#define JADEBASE_VERSION_MINOR   1
#define JADEBASE_VERSION_PATCH   0

namespace
{
    jade::mutex version_mutex;
    
    jade::version jadebase_version = { JADEBASE_VERSION_MAJOR,
                                       JADEBASE_VERSION_MINOR,
                                       JADEBASE_VERSION_PATCH };
    
    std::string program_name;
    jade::version program_version = { 0, 0, 0 };
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    version& getJadebaseVersion()
    {
        return jadebase_version;
    }
    std::string getJadebaseVersionString()
    {
        std::string str;
        
        ff::write( str,
                   "jadebase ",
                   jadebase_version.major,
                   ".",
                   jadebase_version.minor,
                   ".",
                   jadebase_version.patch );
        
        return str;
    }
    
    void setProgramName( std::string n )
    {
        scoped_lock< mutex > slock( version_mutex );
        
        program_name = n;
    }
    std::string getProgramName()
    {
        scoped_lock< mutex > slock( version_mutex );
        
        return program_name;
    }
    void setProgramVersion( version& v )
    {
        scoped_lock< mutex > slock( version_mutex );
        
        program_version = v;
    }
    void setProgramVersion( int major, int minor, int patch )
    {
        scoped_lock< mutex > slock( version_mutex );
        
        program_version.major = major;
        program_version.minor = minor;
        program_version.patch = patch;
    }
    version getProgramVersion()
    {
        scoped_lock< mutex > slock( version_mutex );
        
        return program_version;
    }
    std::string getProgramVersionString()
    {
        scoped_lock< mutex > slock( version_mutex );
        
        std::string str;
        
        ff::write( str,
                   program_name,
                   " ",
                   program_version.major,
                   ".",
                   program_version.minor,
                   ".",
                   program_version.patch );
        
        return str;
    }
}


