/* 
 * jb_threadutil.cpp
 * 
 * Implements jb_threadutil.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_threadutil.hpp"

#if defined PLATFORM_XWS_GNUPOSIX
#include <unistd.h>
#include <stdint.h>
#endif

#include "../jb_exception.hpp"
#include "../jb_log.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    long getSystemCoreCount()
    {
        #if defined PLATFORM_XWS_GNUPOSIX
        return sysconf( _SC_NPROCESSORS_ONLN );
        #else
        // TODO: implement others
        throw exception( "Could not get CPU core count" );
        #endif
    }
    
    std::string exc2str( exit_code ec )
    {
        switch( ( intptr_t )ec )                                                // We know for certain that this is not a real pointer
        {
            case EXIT_FINE:
                return "EXIT_FINE";
            case EXIT_INITERR:
                return "EXIT_INITERR";
            case EXIT_JBERR:
                return "EXIT_JBERR";
            case EXIT_STDERR:
                return "EXIT_STDERR";
            default:
                return "n/a";
        }
    }
    
    #if defined PLATFORM_XWS_GNUPOSIX || defined PLATFORM_MACOSX
    
    std::string errc2str( int err )
    {
        char buff[ 32 ];
        
        #if ( _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 ) && ! _GNU_SOURCE
        
        strerror_r( err, buff, 32 );
        
        return std::string( buff );
        
        #else
        
        return std::string( strerror_r( err, buff, 32 ) );
        
        #endif
    }
    
    #elif defined PLATFORM_WINDOWS
    
    #endif
}


