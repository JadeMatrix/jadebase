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

#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
{
    void openInputDevices();
    void closeInputDevices();
    void refreshInputDevices();
    
    void handleStrokeEvent( XEvent& );
}

/******************************************************************************//******************************************************************************/

#endif


