#ifndef JADEBASE_THREADUTIL_HPP
#define JADEBASE_THREADUTIL_HPP

/* 
 * jb_threadutil.hpp
 * 
 * Utilities file for concurrency wrappers
 * 
 * Contains all the recognized thread exit codes, both as exit_code's and
 * definitions sof main() & switches can use the values.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    typedef void* exit_code;
    typedef exit_code (* thread_func)( void* );
    
    #define EXIT_FINE    0x00
    #define EXIT_INITERR 0x01
    #define EXIT_JBERR   0x02
    #define EXIT_STDERR  0x03
    #define EXIT_ARREST  0x04
    static exit_code EXITCODE_FINE    = ( void* )EXIT_FINE;
    static exit_code EXITCODE_INITERR = ( void* )EXIT_INITERR;
    static exit_code EXITCODE_JBERR   = ( void* )EXIT_JBERR;
    static exit_code EXITCODE_STDERR  = ( void* )EXIT_STDERR;
    static exit_code EXITCODE_ARREST  = ( void* )EXIT_ARREST;
    
    long getSystemCoreCount();
    
    std::string exc2str( exit_code );                                           // "EXit Code 2 [to] STRing"
    
    #if defined PLATFORM_XWS_GNUPOSIX || defined PLATFORM_MACOSX
    
    std::string errc2str( int );                                                // "unix ERRor Code 2 [to] STRing" (UNIX error as an int)
    
    #elif defined PLATFORM_WINDOWS
    
    #endif
}

/******************************************************************************//******************************************************************************/

#endif


