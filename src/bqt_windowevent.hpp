#ifndef BQT_WINDOWEVENT_HPP
#define BQT_WINDOWEVENT_HPP

/* 
 * bqt_windowevent.hpp
 * 
 * Events are for general actions on windows and not on the program as a whole.
 * This means they include clicks, drags, etc. but not file opening, quitting,
 * etc. that affect the program as a whole.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



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
        click_type click;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;                                                         // Apple Option/Alt or Windows Alt
        bool meta  : 1;                                                         // Apple Command or Windows key
        
        float   position[ 2 ];                                                  // Position [ x, y ] relative to screen (fractional if supported)
        float   pressure;                                                       // Pressure, ( 0.0 ... 1.0 )
        int         tilt[ 2 ];                                                  // Tilt [ x, y ], ( -1.0 ... 1.0 )
        float   rotation;                                                       // Rotation (0.0 up to but not including 1.0 is a full rotation, can contain
                                                                                // multiple rotations)
        float      wheel;                                                       // Tangential (wheel) pressure -1.0 through 1.0
    };
    
    struct key_input
    {
        bqt_platform_keycode_t key;
        
        bool shift : 1;
        bool ctrl  : 1;
        bool alt   : 1;
        bool meta  : 1;
    };
    
    struct pinch_input
    {
        float distance;                                                         // Relative change in distance
        float rotation;                                                         // Relative change, 0.0 through 1.0 for a full rotation, repeating
        int position[ 2 ];                                                      // Absolute position in-window
    };
    
    // WINDOW EVENT ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    enum wevent_id
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
        wevent_id id;
        union
        {
            stroke_waypoint stroke;
            //drop;
            key_input key;
            //command;
            //text;
            pinch_input pinch;
        };
    };
    
    // enum wevent_id
    // {
    //     CLICK,
    //     DRAG,
    //     DROP,
    //     COMMAND,
    //     TEXT
    // };
    
    // // TODO: Very much a work in progress
    // struct window_event
    // {
    //     wevent_id id;
    //     union
    //     {
    //         struct
    //         {
    //             unsigned int position[ 2 ];
    //         } click;
    //         struct
    //         {
    //             unsigned int start[ 2 ];
    //             unsigned int end[ 2 ];
    //         } drag;
    //         struct
    //         {
    //             unsigned int position[ 2 ];
    //         } drop;
    //         struct
    //         {
                
    //         } command;
    //         struct
    //         {
                
    //         } text;
    //     } data;
    // };
}

/******************************************************************************//******************************************************************************/

#endif

