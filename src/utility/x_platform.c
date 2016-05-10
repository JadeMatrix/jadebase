/* 
 * x_platform.c
 * 
 * Implementation of jb_platform.h for POSIX + GNU + X Window System
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_platform.h"

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


