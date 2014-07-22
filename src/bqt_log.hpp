#ifndef BQT_LOG_HPP
#define BQT_LOG_HPP

/* 
 * bqt_log.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>

#include <fastformat/sinks/ostream.hpp>
#include <fastformat/shims/conversion/filter_type/reals.hpp>
#include <fastformat/shims/conversion/filter_type/bool.hpp>
#include <fastformat/shims/conversion/filter_type/void_pointers.hpp>
#include <fastformat/ff.hpp>
#ifdef FASTFORMAT_NO_FILTER_TYPE_CONVERSION_SHIM_SUPPORT
# error "FastFormat: Cannot compile this file with a compiler that does not support the filter_type mechanism"
#endif

/******************************************************************************//******************************************************************************/

#define MACROTOSTR_A( D ) #D
#define   MACROTOSTR( D ) MACROTOSTR_A( D )                                     // Double expansion trick

namespace bqt
{
    std::ostream& getLogStream();                                               // Returns the std::ostream being used as a log; std::cout by default
    #define bqt_out bqt::getLogStream()                                         // Macro so we can get the current out stream like ff::write( bqt_out, "...
    
    void closeLog();                                                            // To be called on exit (in main()) to make sure we've cleaned up logging
}

/******************************************************************************//******************************************************************************/

#endif


