#ifndef JADEBASE_TIMESTAMP_HPP
#define JADEBASE_TIMESTAMP_HPP

/* 
 * jb_timestamp.hpp
 * 
 * Contains utilities for retrieving timestamps.
 * 
 * getTrackTimestamp() returns an internal timestamp for change tracking; this
 * may or may not be the same as getUNIXTimestamp().
 * 
 * getUNIXTimestamp() is essentially C's time( NULL ): it returns the non-local
 * time in seconds since 1/1/70.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <ctime>

/******************************************************************************//******************************************************************************/

namespace jade
{
    typedef unsigned long timestamp;
    
    // timestamp getTrackTimestamp();
    
    timestamp getUNIXTimestamp();
}

/******************************************************************************//******************************************************************************/

#endif


