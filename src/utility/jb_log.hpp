#ifndef JADEBASE_LOG_HPP
#define JADEBASE_LOG_HPP

/* 
 * jb_log.hpp
 * 
 * Utilities for handling log output
 * 
 * While log output can be redirected to a file (see jb_launchargs.cpp), the
 * recommended method is to keep log output directed to std::cout and launch the
 * program from a script that pipes all output to a crash log.  This guarantees
 * that any stray uncaught exceptions still get logged.
 * 
 * Logging uses FastFormat for formatting; the documentation can be found here:
 * http://fastformat.sourceforge.net/documentation/index.html
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>

#include <fastformat/inserters/ch.hpp>                                          // Because FastFormat can't handle basic chars for some reason
#include <fastformat/inserters/to_x.hpp>
#include <fastformat/shims/conversion/filter_type/bool.hpp>
#include <fastformat/shims/conversion/filter_type/reals.hpp>
#include <fastformat/shims/conversion/filter_type/void_pointers.hpp>
#include <fastformat/sinks/ostream.hpp>

#include <fastformat/ff.hpp>

#ifdef FASTFORMAT_NO_FILTER_TYPE_CONVERSION_SHIM_SUPPORT
#error "FastFormat: Cannot compile this file with a compiler that does not support the filter_type mechanism"
#endif

/******************************************************************************//******************************************************************************/

#define MACROTOSTR_A( D ) #D
#define   MACROTOSTR( D ) MACROTOSTR_A( D )                                     // Double expansion trick

#define PTR_HEX_WIDTH   ( ( int )( sizeof( void* ) * 2 ) )

namespace jade
{
    std::ostream& getLogStream();                                               // Returns the std::ostream being used as a log; std::cout by default
    #define jb_out jade::getLogStream()                                         // Macro so we can get the current out stream as ff::write( jb_out, "...
    
    void closeLog();                                                            // To be called on exit (in main()) to make sure we've cleaned up logging
}

/******************************************************************************//******************************************************************************/

#endif


