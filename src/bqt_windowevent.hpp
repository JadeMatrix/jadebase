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
    enum wevent_id
    {
        CLICK,
        DRAG,
        DROP,
        COMMAND,
        TEXT
    };
    
    // TODO: Very much a work in progress
    struct window_event
    {
        wevent_id id;
        union
        {
            struct
            {
                unsigned int position[ 2 ];
            } click;
            struct
            {
                unsigned int start[ 2 ];
                unsigned int end[ 2 ];
            } drag;
            struct
            {
                unsigned int position[ 2 ];
            } drop;
            struct
            {
                
            } command;
            struct
            {
                
            } text;
        } data;
    };
}

/******************************************************************************//******************************************************************************/

#endif


