/* 
 * bqt_platform.c
 * 
 * Implements utility functions from bqt_platform.h
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_platform.h"

/******************************************************************************//******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
    
    int bqt_platform_idevid_t_comp( const bqt_platform_idevid_t left,
                                    const bqt_platform_idevid_t right )
    {
        if( left.x_real_dev == right.x_real_dev )                               // bqt_platform_idevid_t::x_real_dev is a bool, so this works as XNOR
        {
            return left.x_devid < right.x_devid;
        }
        else
        {
            if( left.x_real_dev )                                               // Real Xlib device IDs are considered 'greater than' dummy IDs
                return 0;
            else
                return 1;
        }
    }
    
    int bqt_platform_idevid_t_equal( const bqt_platform_idevid_t a,
                                     const bqt_platform_idevid_t b )
    {
        return ( a.x_real_dev == b.x_real_dev ) && ( a.x_devid == b.x_devid );
    }
    
#ifdef __cplusplus
}
#endif


