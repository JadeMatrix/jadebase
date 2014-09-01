#ifndef X_INPUTDEVICES_HPP
#define X_INPUTDEVICES_HPP

/* 
 * x_inputdevices.hpp
 * 
 * Utilities for recognizing & interpreting X input devices & associated events
 * 
 */

#ifndef PLATFORM_XWS_GNUPOSIX
#error PLATFORM_XWS_GNUPOSIX not defined
#endif

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_platform.h"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // struct idev_description
    // {
    //     enum
    //     {
    //         OTHER,
    //         MOUSE,
    //         TOUCH,
    //         STYLUS
    //     } type;
        
    //     std::string name;
        
    //     union
    //     {
    //         struct
    //         {
                
    //         } mouse;
    //         struct
    //         {
                
    //         } touch;
    //         struct
    //         {
    //             pen_type type;
    //             XAxisInfo axes[ 6 ];
    //             bqt_platform_idevid_t x_devid;
    //             XDevice* x_device;
    //             int x_motioneventtype;
    //         } stylus;
    //     };
    // };
    
    void openInputDevices();
    void closeInputDevices();
    void refreshInputDevices();
    
    void handleStrokeEvent( XEvent& x_event );
}

/******************************************************************************//******************************************************************************/

#endif


