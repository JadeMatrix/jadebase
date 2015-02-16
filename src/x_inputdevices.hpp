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
    void openInputDevices();
    void closeInputDevices();
    void refreshInputDevices();
    
    void handleStrokeEvent( XEvent& x_event );
}

/******************************************************************************//******************************************************************************/

#endif


