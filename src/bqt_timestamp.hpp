#ifndef BQT_TIMESTAMP_HPP
#define BQT_TIMESTAMP_HPP

/* 
 * bqt_timestamp.hpp
 * 
 * Contains utilities for retrieving timestamps.
 * 
 * getTrackTimestamp() returns an internal timestamp for change tracking; this
 * may or may not be the same as getUNIXTimestamp().
 * 
 * getUNIXTimestamp() is essentially C's time( NULL ): it returrns the non-local
 * time in seconds since 1/1/70.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <ctime>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    typedef unsigned long timestamp;
    
    timestamp getTrackTimestamp();
    
    timestamp getUNIXTimestamp();
}

/******************************************************************************//******************************************************************************/

#endif


