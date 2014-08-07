#ifndef BQT_WINDOWEVENT_HPP
#define BQT_WINDOWEVENT_HPP

/* 
 * bqt_windowevent.hpp
 * 
 * Events are for general actions on windows and not on the program as a whole.
 * This means they include clicks, drags, etc. but not file opening, quitting,
 * etc. that affect the program as a whole.
 * 
 * Note about bqt::stroke_waypoint::cmd - this flag is set if the system-depend-
 * ant application command key is pressed.  On OS X, this is the Command key
 * (which is also used for system shortcuts).  On Windows & Linux, this is CTRL,
 * as the super key is used for system-level shortcuts.  The appropriate flag
 * bqt::stroke_waypoint::ctrl or bqt::stroke_waypoint::super will also be set.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include "bqt_keycode.hpp"
#include "bqt_platform.h"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // WINDOW EVENT TYPES //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    // http://www.wacomeng.com/mac/Developers%20Guide.htm
    
    typedef unsigned short click_type;
    #define CLICK_PRIMARY   0x0001      // 0000 0001
    #define CLICK_SECONDARY 0x0002      // 0000 0010
    #define CLICK_ALT       0x0004      // 0000 0100
    #define CLICK_ERASE     0x0008      // 0000 1000
    #define CLICK_LENS      0x0010      // 0001 0000
    
    struct stroke_waypoint
    {
        bqt_platform_idevid_t dev_id;
        
        click_type click;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;                                                         // Apple Option/Alt or Windows Alt
        bool super : 1;                                                         // Apple Command or Windows key
        bool cmd   : 1;                                                         // Platform command key: CTRL on Windows/Linux, Command on OS X
        
        float   position[ 2 ];                                                  // Position [ x, y ] relative to screen (fractional if supported)
        float   pressure;                                                       // Pressure, ( 0.0 ... 1.0 )
        int         tilt[ 2 ];                                                  // Tilt [ x, y ], ( -1.0 ... 1.0 )
        float   rotation;                                                       // Rotation (0.0 up to but not including 1.0 is a full rotation, can contain
                                                                                // multiple rotations but usually not)
        float      wheel;                                                       // Tangential (wheel) pressure -1.0 through 1.0
    };
    
    struct drop_item
    {
        int position[ 0 ];
        // droppable* item;
    };
    
    struct key_command
    {
        keycode key;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;
        bool super : 1;
        bool cmd   : 1;
        
        bool up;
    };
    std::string getKeyCommandString( key_command& k );                          // TODO: Make this UTF-8 to return symbols for modifier keys
    
    struct command
    {
        enum
        {
            
        } type;
        union
        {
            
        } data;
    };
    
    class layout_element;
    struct text_input
    {
        layout_element* element;                                                // Text input always occurst after a layout_element requests it
        std::string* utf8str;                                                   // Unfortunately have to use a string* here
    };
    
    struct pinch_input
    {
        bqt_platform_idevid_t dev_id;
        
        bool finish;
        
        float distance;                                                         // Relative change in distance
        float rotation;                                                         // Relative change, 0.0 through 1.0 for a full rotation, repeating
        int position[ 2 ];                                                      // Absolute position in-window
    };
    
    // WINDOW EVENT ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    enum wevent_type
    {
        NONE,
        STROKE,
        DROP,
        KEYCOMMAND,
        COMMAND,
        TEXT,
        PINCH
    };
    
    struct window_event
    {
        wevent_type type;
        union
        {
            stroke_waypoint stroke;
            drop_item drop;
            key_command key;
            command cmd;
            text_input text;
            pinch_input pinch;
        };
    };
}

/******************************************************************************//******************************************************************************/

#endif


