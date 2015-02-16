/* 
 * jb_windowevent.cpp
 * 
 * Implements wevent3str() from jb_windowevent.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_windowevent.hpp"

#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    std::string wevent2str( window_event& e )
    {
        std::string str;
        
        switch( e.type )
        {
        case NONE:
            return "NONE type";
            break;
        case STROKE:
            {
                ff::write( str,
                           "STROKE from device 0x",
                           ff::to_x( e.stroke.dev_id, 2, 2 ),
                           ": " );
                
                if( e.stroke.click & CLICK_PRIMARY )
                    ff::write( str, "CLICK_PRIMARY " );
                if( e.stroke.click & CLICK_SECONDARY )
                    ff::write( str, "CLICK_SECONDARY " );
                if( e.stroke.click & CLICK_ALT )
                    ff::write( str, "CLICK_ALT " );
                if( e.stroke.click & CLICK_ERASE )
                    ff::write( str, "CLICK_ERASE " );
                if( e.stroke.click & CLICK_LENS )
                    ff::write( str, "CLICK_LENS " );
                
                if( e.stroke.shift )
                    ff::write( str, "Shift " );
                if( e.stroke.ctrl )
                    ff::write( str, "Ctrl " );
                if( e.stroke.alt )
                    ff::write( str, "Alt " );
                if( e.stroke.super )
                    ff::write( str, "Super " );
                if( e.stroke.cmd )
                    ff::write( str, "(command) " );
                
                ff::write( str,
                           "@ ",
                           e.stroke.position[ 0 ],
                           ",",
                           e.stroke.position[ 1 ],
                           " (",
                           e.stroke.position[ 0 ] - e.offset[ 0 ],
                           ",",
                           e.stroke.position[ 1 ] - e.offset[ 1 ],
                           ")  press ",
                           e.stroke.pressure,
                           " tilt ",
                           e.stroke.tilt[ 0 ],
                           ",",
                           e.stroke.tilt[ 1 ],
                           " rotate ",
                           e.stroke.rotation,
                           " wheel ",
                           e.stroke.wheel );
            }
            break;
        case DROP:
            ff::write( str,
                       "DROP at ",
                       e.drop.position[ 0 ],
                       ",",
                       e.drop.position[ 1 ],
                       " with item 0x",
            #warning DROP event to string not fully implemented
                       ff::to_x( 0x00, 2, 2 ) );
            break;
        case KEYCOMMAND:
            ff::write( str,
                       "KEYCOMMAND ",
                       getKeyCommandString( e.key ),
                       ( e.key.up ? " (Up)" : " (Down)" ) );
            break;
        case COMMAND:
            #warning COMMAND event to string not fully implemented
            ff::write( str, "COMMAND" );
            break;
        case TEXT:
            if( e.text.utf8str == NULL )
                ff::write( str, "TEXT with null string" );
            else
                ff::write( str, "TEXT \"", *e.text.utf8str, "\"" );
            break;
        case PINCH:
            ff::write( str,
                       "PINCH ",
                       ( e.pinch.finish ? "(finish) " : "" ),
                       "@ ",
                       e.pinch.position[ 0 ],
                       ",",
                       e.pinch.position[ 1 ],
                       " (",
                       e.stroke.position[ 0 ] - e.offset[ 0 ],
                       ",",
                       e.stroke.position[ 1 ] - e.offset[ 1 ],
                       ")  distance ",
                       e.pinch.distance,
                       " rotation ",
                       e.pinch.rotation );
            break;
        case SCROLL:
            {
                ff::write( str,
                           "SCROLL: " );
                
                if( e.scroll.shift )
                    ff::write( str, "Shift " );
                if( e.scroll.ctrl )
                    ff::write( str, "Ctrl " );
                if( e.scroll.alt )
                    ff::write( str, "Alt " );
                if( e.scroll.super )
                    ff::write( str, "Super " );
                if( e.stroke.cmd )
                    ff::write( str, "(command) " );
                
                ff::write( str,
                           "@ ",
                           e.scroll.position[ 0 ],
                           ",",
                           e.scroll.position[ 1 ],
                           " (",
                           e.stroke.position[ 0 ] - e.offset[ 0 ],
                           ",",
                           e.stroke.position[ 1 ] - e.offset[ 1 ],
                           ")  vertical ",
                           e.scroll.amount[ 1 ],
                           " horizontal ",
                           e.scroll.amount[ 0 ] );
            }
            break;
        default:
            ff::write( str, "Invalid type" );
            break;
        }
        
        return str;
    }
}


