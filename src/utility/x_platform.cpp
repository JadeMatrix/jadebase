/* 
 * x_platform.cpp
 * 
 * Implementation of jb_platform.h for POSIX + GNU + X Window System
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_platform.h"

#include "jb_log.hpp"

/******************************************************************************//******************************************************************************/

int jb_platform_window_t_compare( const jb_platform_window_t left,
                                  const jb_platform_window_t right )
{
    return ( int )( left.x_window - right.x_window );
}

int jb_platform_idevid_t_compare( const jb_platform_idevid_t left,
                                  const jb_platform_idevid_t right )
{
    return ( int )( left - right );
}

namespace jade
{
    std::string jb_platform_idevid_t_2str( const jb_platform_idevid_t& dev_id )
    {
        std::string str;
        ff::write( str, "0x", ff::to_x( dev_id, 2, 2 ) );
        return str;
    }
}


