#ifndef BQT_TIMESTAMP_HPP
#define BQT_TIMESTAMP_HPP

/* 
 * bqt_timestamp.hpp
 * 
 * Contains utilities for retrieving timestamps.
 * 
 * getUNIXTimestamp() is essentially C's time( NULL ), but may be implementa-
 * tion-defined.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <ctime>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    typedef unsigned long timestamp;
    
    timestamp getBlockTimestamp();                                              // Return internal timestamp for block updates
    
    timestamp getUNIXTimestamp();                                               // Return the local time in seconds since 1/1/70
}

/******************************************************************************//******************************************************************************/

#endif


