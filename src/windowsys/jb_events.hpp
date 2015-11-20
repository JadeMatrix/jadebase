#ifndef JADEBASE_EVENTS_HPP
#define JADEBASE_EVENTS_HPP

/* 
 * jb_events.hpp
 * 
 * jadebase makes reasonable effort to support the various features of disparate
 * hardware input devices in some kind of unified way in the light of this
 * software's functionality, and as supported by the platforms it runs on. Any
 * such support/conversions will make as few assumptions as possible; any such
 * assumptions should be documented.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



/******************************************************************************//******************************************************************************/

namespace jade
{
    void startEventSystem();                                                    // Not thread-safe
    void stopEventSystem();
}

/******************************************************************************//******************************************************************************/

#endif


