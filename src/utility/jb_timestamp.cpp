/* 
 * jb_timestamp.cpp
 * 
 * Implements timestamp retrieval functions from jb_timestamp.hpp
 * 
 * There is no wraparound safety for getBlockTimestamp() because even assuming
 * the user performs an action that would require a block update once every
 * single millisecond on a machine with 32-bit longs, that still means they can
 * work for nearly 50 days straight.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_timestamp.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    // jade::timestamp internal_timestamp_counter = 0;
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    // timestamp getBlockTimestamp()
    // {
    //     return internal_timestamp_counter++;                                    // Return the counter & increment
    // }
    
    timestamp getUNIXTimestamp()
    {
        return ( timestamp )time( NULL );                                       // Conversion can be done later using localtime if necessary
    }
}


