#ifndef JADEBASE_WINDOWEVENT_HPP
#define JADEBASE_WINDOWEVENT_HPP

/* 
 * jb_windowevent.hpp
 * 
 * Events are for general actions on windows and not on the program as a whole.
 * This means they include clicks, drags, etc. but not file opening, quitting,
 * etc. that affect the program as a whole.
 * 
 * Note about jade::stroke_waypoint::cmd - this flag is set if the system-
 * dependant application command key is pressed.  On OS X, this is the Command
 * key (which is also used for system shortcuts).  On Windows & Linux, this is
 * CTRL, as the super key is used for system-level shortcuts.  The appropriate
 * flag jade::stroke_waypoint::ctrl or jade::stroke_waypoint::super will also be
 * set.
 * 
 */

// TODO: Check to see if float parameters in stroke_waypoint should be changed to double for appropriate accuracy / future-proofing

/* INCLUDES *******************************************************************//******************************************************************************/

#include <cmath>
#include <string>

#include "jb_keycode.hpp"
#include "../utility/jb_dpi.hpp"
#include "../utility/jb_platform.h"

/******************************************************************************//******************************************************************************/

namespace jade
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
        jb_platform_idevid_t dev_id;
        
        click_type click;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;                                                         // Apple Option/Alt or Windows Alt
        bool super : 1;                                                         // Apple Command or Windows key
        bool cmd   : 1;                                                         // Platform command key: CTRL on Windows/Linux, Command on OS X
        
        dpi::points prev_pos[ 2 ];                                              // Previous position [ x, y ]; { NaN, NaN } if no previous
        float       pressure;                                                   // Pressure; ( 0.0 ... 1.0 )
        float           tilt[ 2 ];                                              // Tilt [ x, y ]; ( -1.0 ... 1.0 )
        float       rotation;                                                   // Rotation (0.0 up to but not including 1.0 is a full rotation, can contain
                                                                                // multiple rotations but usually not)
        float      wheel;                                                       // Tangential (wheel) pressure -1.0 through 1.0
        
        // float      speed;                                                       // Speed scalar in mm/s
    };
    
    // class droppable;
    struct drop_item
    {
        void* foo;
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
    
    class gui_element;
    struct text_input
    {
        gui_element* element;                                                   // Text input always occurs after a gui_element requests it
        std::string* utf8str;                                                   // Unfortunately have to use a string* here
    };
    
    struct pinch_input
    {
        jb_platform_idevid_t dev_id;
        
        bool finish;
        
        dpi::points distance;                                                   // Relative change in distance
        float       rotation;                                                   // Relative change, 0.0 through 1.0 for a full rotation, repeating
    };
    
    struct scroll_input
    {
        dpi::points amount[ 2 ];                                                // Distance scrolled in points on each axis
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;
        bool super : 1;
        bool cmd   : 1;
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
        PINCH,
        SCROLL
    };
    
    struct window_event
    {
        wevent_type type;
        dpi::points position[ 2 ];                                              // Position [ x, y ]; { NaN, NaN } if meaningless for given event type
        union
        {
            stroke_waypoint stroke;
            drop_item drop;
            key_command key;
            command cmd;
            text_input text;
            pinch_input pinch;
            scroll_input scroll;
        };
        
        window_event()
        {
            type = NONE;
            position[ 0 ] = NAN;
            position[ 1 ] = NAN;
        }
    };
    
    // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::string wevent2str( window_event& );                                    // Window EVENT 2 [to] STRing
    
    #define pointInsideRect( p_x, p_y, r_x, r_y, r_w, r_h ) ( p_x >= r_x\
                                                              && p_y >= r_y\
                                                              && p_x <  r_x + r_w\
                                                              && p_y <  r_y + r_h ) // We assume the compiler can optimize this a bit
    
    #define pointInsideCircle( p_x, p_y, c_x, c_y, c_r ) ( ( p_x - c_x ) * ( p_x - c_x ) + ( p_y - c_y ) * ( p_y - c_y )\
                                                            <= c_r * c_r )      // c_x & c_y are the center of the circle
}

/******************************************************************************//******************************************************************************/

#endif


