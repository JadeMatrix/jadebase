/* 
 * jb_scrollset.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_scrollset.hpp"

#include "jb_named_resources.hpp"
#include "jb_group.hpp"
#include "jb_resource.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_settings.hpp"
#include "../windowsys/jb_window.hpp"

#include "../utility/jb_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex scroll_rsrc_mutex;
    bool got_resources = false;
    
    struct button_set
    {
        jade::gui_resource* up;
        jade::gui_resource* down;
    };
    
    struct bar_set
    {
        button_set left_top;
        button_set fill;
        button_set right_bottom;
    };
    
    struct corner_set
    {
        jade::gui_resource* up;
        jade::gui_resource* down;
        jade::gui_resource* evil;
    };
    
    struct
    {
        button_set left_top;
        button_set right_bottom;
        
        bar_set bar;
        
        jade::gui_resource* fill;
        
        corner_set corner;
    } scroll_set;
}

/******************************************************************************//******************************************************************************/

#define SCROLLBAR_HEIGHT                ( ( dpi::points )12 )
#define MIN_SCROLLBAR_LENGTH            ( ( dpi::points )12 )
#define SCROLLBAR_BUTTON_REAL_WIDTH     ( ( dpi::points )18 )                   // Width of the button bounds
#define SCROLLBAR_BUTTON_VISUAL_WIDTH   ( ( dpi::points )25 )                   // Width of the button sprite
#define SLIDER_END_WIDTH                ( ( dpi::points )6  )

// #define DEPRESSABLE_SLIDER_BARS

namespace jade
{
    scrollset::scrollset( dpi::points x,
                          dpi::points y,
                          dpi::points w,
                          dpi::points h,
                          const std::shared_ptr< scrollable >& c )
                        : gui_element( x, y, w, h ),
                          contents( c )
    {
        init();
        
        contents -> setParentElement( this );
        contents -> setRealDimensions( dimensions[ 0 ],
                                       dimensions[ 1 ] );
        
        arrangeBars();
    }
    scrollset::scrollset( dpi::points x,
                          dpi::points y,
                          const std::shared_ptr< scrollable >& c )
                        : gui_element( x, y, 0, 0 ),
                          contents( c )
    {
        init();
        
        std::pair< dpi::points, dpi::points > c_dims( contents -> getRealDimensions() );
        
        setRealDimensions( c_dims.first, c_dims.second );
        
        contents -> setParentElement( this );
        
        arrangeBars();
    }
    scrollset::~scrollset()
    {
        // Empty
    }
    
    void scrollset::setRealPosition( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void scrollset::setRealDimensions( dpi::points w, dpi::points h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        ff::write( jb_out,
                   ">>> Setting scrollset dimensions to ",
                   w,
                   " x ",
                   h,
                   "\n" );
        
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT );
        
        arrangeBars();                                                          // May call parent -> requestRedraw()
    }
    
    void scrollset::setBarsAlwaysVisible( bool v )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        bars_always_visible = v;
        
        arrangeBars();                                                          // May call parent -> requestRedraw()
    }
    bool scrollset::getBarsAlwaysVisible()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return bars_always_visible;
    }
    
    bool scrollset::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // Set up area flags ///////////////////////////////////////////////////
        
        enum area
        {
            IN_CONTENTS            = 0x800,     // 0000 1000 0000 0000
            
            IN_CONTROL_AREA        = 0x7FF,     // 0000 0111 1111 1111 - meta
            
            IN_CORNER_AREA         = 0x400,     // 0000 0100 0000 0000
            
            IN_BOTTOM_AREA         = 0x01F,     // 0000 0000 0001 1111 - meta
            IN_BOTTOM_LEFT_BUTTON  = 0x010,     // 0000 0000 0001 0000
            IN_BOTTOM_LEFT_SPACE   = 0x008,     // 0000 0000 0000 1000
            IN_BOTTOM_BAR          = 0x004,     // 0000 0000 0000 0100
            IN_BOTTOM_RIGHT_SPACE  = 0x002,     // 0000 0000 0000 0010
            IN_BOTTOM_RIGHT_BUTTON = 0x001,     // 0000 0000 0000 0001
            
            IN_SIDE_AREA           = 0x3E0,     // 0000 0011 1110 0000 - meta
            IN_SIDE_TOP_BUTTON     = 0x200,     // 0000 0010 0000 0000
            IN_SIDE_TOP_SPACE      = 0x100,     // 0000 0001 0000 0000
            IN_SIDE_BAR            = 0x080,     // 0000 0000 1000 0000
            IN_SIDE_BOTTOM_SPACE   = 0x040,     // 0000 0000 0100 0000
            IN_SIDE_BOTTOM_BUTTON  = 0x020,     // 0000 0000 0010 0000
            
            IN_ANYWHERE            = 0xFFF,     // 0000 1111 1111 1111 - meta
            IN_NOWHERE             = 0x000      // 0000 0000 0000 0000 - init
        };
        
        area place = IN_NOWHERE;
        area prev_place = IN_NOWHERE;
        
        switch( e.type )
        {
        case STROKE:
            if( !pointInsideRect( e.stroke.prev_pos[ 0 ],
                                  e.stroke.prev_pos[ 1 ],
                                  0,
                                  0,
                                  dimensions[ 0 ],
                                  dimensions[ 1 ] ) )                           // Break early if not in scrollset bounds
            {
                goto rest;                                                      // Can't break quite yet
            }

            if( e.stroke.prev_pos[ 0 ] < dimensions[ 0 ] - SCROLLBAR_HEIGHT )          // Left of the vertical scrollbar
            {
                if( e.stroke.prev_pos[ 1 ] < dimensions[ 1 ] - SCROLLBAR_HEIGHT )      // Outside of both bars so inside contents
                    prev_place = IN_CONTENTS;
                else
                {
                    if( pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                           e.stroke.prev_pos[ 1 ],
                                           slider_pos[ 0 ] + SCROLLBAR_HEIGHT / 2,
                                           dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                           SCROLLBAR_HEIGHT / 2 )
                        || pointInsideRect( e.stroke.prev_pos[ 0 ],
                                            e.stroke.prev_pos[ 1 ],
                                            slider_pos[ 0 ] + SCROLLBAR_HEIGHT / 2,
                                            dimensions[ 1 ] - SCROLLBAR_HEIGHT, // This could also be 0 as we already checked not above
                                            slider_width[ 0 ] - SCROLLBAR_HEIGHT,
                                            SCROLLBAR_HEIGHT )
                        || pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                              e.stroke.prev_pos[ 1 ],
                                              slider_pos[ 0 ] + slider_width[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                              dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                              SCROLLBAR_HEIGHT / 2 ) )          // Inside scroll bar handle
                    {
                        prev_place = IN_BOTTOM_BAR;
                    }
                    else
                        if( e.stroke.prev_pos[ 0 ]
                            < slider_pos[ 0 ] + slider_width[ 0 ] / 2 )         // Left hand side of bottom area
                        {
                            if( e.stroke.prev_pos[ 0 ] < SCROLLBAR_BUTTON_VISUAL_WIDTH
                                && !pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                                       e.stroke.prev_pos[ 1 ],
                                                       SCROLLBAR_BUTTON_VISUAL_WIDTH,
                                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside bottom left button
                            {
                                prev_place = IN_BOTTOM_LEFT_BUTTON;
                            }
                            else
                                prev_place = IN_BOTTOM_LEFT_SPACE;
                        }
                        else                                                    // Right hand side of bottom area
                        {
                            if( e.stroke.prev_pos[ 0 ] >= dimensions[ 0 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT )
                                && !pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                                       e.stroke.prev_pos[ 1 ],
                                                       dimensions[ 0 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT ),
                                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside bottom right button
                            {
                                prev_place = IN_BOTTOM_RIGHT_BUTTON;
                            }
                            else
                                prev_place = IN_BOTTOM_RIGHT_SPACE;
                        }
                }
            }
            else                                                                // (Potentially) in the vertical scrollbar
            {
                if( e.stroke.prev_pos[ 1 ] < dimensions[ 1 ] - SCROLLBAR_HEIGHT )
                {
                    if( pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                           e.stroke.prev_pos[ 1 ],
                                           dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                           slider_pos[ 1 ] + SCROLLBAR_HEIGHT / 2,
                                           SCROLLBAR_HEIGHT / 2 )
                        || pointInsideRect( e.stroke.prev_pos[ 0 ],
                                            e.stroke.prev_pos[ 1 ],
                                            dimensions[ 0 ] - SCROLLBAR_HEIGHT, // This could also be 0 as we already checked not left
                                            slider_pos[ 1 ] + SCROLLBAR_HEIGHT / 2,
                                            SCROLLBAR_HEIGHT,
                                            slider_width[ 1 ] - SCROLLBAR_HEIGHT )
                        || pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                              e.stroke.prev_pos[ 1 ],
                                              dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                              slider_pos[ 1 ] + slider_width[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                              SCROLLBAR_HEIGHT / 2 ) )          // Inside scroll bar handle
                    {
                        prev_place = IN_SIDE_BAR;
                    }
                    else
                        if( e.stroke.prev_pos[ 1 ]
                            < slider_pos[ 1 ] + slider_width[ 1 ] / 2 )         // Top of side area
                        {
                            if( e.stroke.prev_pos[ 1 ] < SCROLLBAR_BUTTON_VISUAL_WIDTH
                                && !pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                                       e.stroke.prev_pos[ 1 ],
                                                       dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_BUTTON_VISUAL_WIDTH,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside side top button
                            {
                                prev_place = IN_SIDE_TOP_BUTTON;
                            }
                            else
                                prev_place = IN_SIDE_TOP_SPACE;
                        }
                        else                                                    // Bottom of side area
                        {
                            if( e.stroke.prev_pos[ 1 ] >= dimensions[ 1 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT )
                                && !pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                                       e.stroke.prev_pos[ 1 ],
                                                       dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                                       dimensions[ 1 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT ),
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside side bottom button
                            {
                                prev_place = IN_SIDE_BOTTOM_BUTTON;
                            }
                            else
                                prev_place = IN_SIDE_BOTTOM_SPACE;
                        }
                }
                else
                    prev_place = IN_CORNER_AREA;
            }
            
        rest:
        case SCROLL:
            if( !pointInsideRect( e.position[ 0 ],
                                  e.position[ 1 ],
                                  0,
                                  0,
                                  dimensions[ 0 ],
                                  dimensions[ 1 ] ) )                           // Break early if not in scrollset bounds
            {
                break;
            }
            
            if( e.position[ 0 ] < dimensions[ 0 ] - SCROLLBAR_HEIGHT )          // Left of the vertical scrollbar
            {
                if( e.position[ 1 ] < dimensions[ 1 ] - SCROLLBAR_HEIGHT )      // Outside of both bars so inside contents
                    place = IN_CONTENTS;
                else
                {
                    if( pointInsideCircle( e.position[ 0 ],
                                           e.position[ 1 ],
                                           slider_pos[ 0 ] + SCROLLBAR_HEIGHT / 2,
                                           dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                           SCROLLBAR_HEIGHT / 2 )
                        || pointInsideRect( e.position[ 0 ],
                                            e.position[ 1 ],
                                            slider_pos[ 0 ] + SCROLLBAR_HEIGHT / 2,
                                            dimensions[ 1 ] - SCROLLBAR_HEIGHT, // This could also be 0 as we already checked not above
                                            slider_width[ 0 ] - SCROLLBAR_HEIGHT,
                                            SCROLLBAR_HEIGHT )
                        || pointInsideCircle( e.position[ 0 ],
                                              e.position[ 1 ],
                                              slider_pos[ 0 ] + slider_width[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                              dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                              SCROLLBAR_HEIGHT / 2 ) )          // Inside scroll bar handle
                    {
                        place = IN_BOTTOM_BAR;
                    }
                    else
                        if( e.position[ 0 ]
                            < slider_pos[ 0 ] + slider_width[ 0 ] / 2 )         // Left hand side of bottom area
                        {
                            if( e.position[ 0 ] < SCROLLBAR_BUTTON_VISUAL_WIDTH
                                && !pointInsideCircle( e.position[ 0 ],
                                                       e.position[ 1 ],
                                                       SCROLLBAR_BUTTON_VISUAL_WIDTH,
                                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside bottom left button
                            {
                                place = IN_BOTTOM_LEFT_BUTTON;
                            }
                            else
                                place = IN_BOTTOM_LEFT_SPACE;
                        }
                        else                                                    // Right hand side of bottom area
                        {
                            if( e.position[ 0 ] >= dimensions[ 0 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT )
                                && !pointInsideCircle( e.position[ 0 ],
                                                       e.position[ 1 ],
                                                       dimensions[ 0 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT ),
                                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside bottom right button
                            {
                                place = IN_BOTTOM_RIGHT_BUTTON;
                            }
                            else
                                place = IN_BOTTOM_RIGHT_SPACE;
                        }
                }
            }
            else                                                                // (Potentially) in the vertical scrollbar
            {
                if( e.position[ 1 ] < dimensions[ 1 ] - SCROLLBAR_HEIGHT )
                {
                    if( pointInsideCircle( e.position[ 0 ],
                                           e.position[ 1 ],
                                           dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                           slider_pos[ 1 ] + SCROLLBAR_HEIGHT / 2,
                                           SCROLLBAR_HEIGHT / 2 )
                        || pointInsideRect( e.position[ 0 ],
                                            e.position[ 1 ],
                                            dimensions[ 0 ] - SCROLLBAR_HEIGHT, // This could also be 0 as we already checked not left
                                            slider_pos[ 1 ] + SCROLLBAR_HEIGHT / 2,
                                            SCROLLBAR_HEIGHT,
                                            slider_width[ 1 ] - SCROLLBAR_HEIGHT )
                        || pointInsideCircle( e.position[ 0 ],
                                              e.position[ 1 ],
                                              dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                              slider_pos[ 1 ] + slider_width[ 1 ] - SCROLLBAR_HEIGHT / 2,
                                              SCROLLBAR_HEIGHT / 2 ) )          // Inside scroll bar handle
                    {
                        place = IN_SIDE_BAR;
                    }
                    else
                        if( e.position[ 1 ]
                            < slider_pos[ 1 ] + slider_width[ 1 ] / 2 )         // Top of side area
                        {
                            if( e.position[ 1 ] < SCROLLBAR_BUTTON_VISUAL_WIDTH
                                && !pointInsideCircle( e.position[ 0 ],
                                                       e.position[ 1 ],
                                                       dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                                       SCROLLBAR_BUTTON_VISUAL_WIDTH,
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside side top button
                            {
                                place = IN_SIDE_TOP_BUTTON;
                            }
                            else
                                place = IN_SIDE_TOP_SPACE;
                        }
                        else                                                    // Bottom of side area
                        {
                            if( e.position[ 1 ] >= dimensions[ 1 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT )
                                && !pointInsideCircle( e.position[ 0 ],
                                                       e.position[ 1 ],
                                                       dimensions[ 0 ] - SCROLLBAR_HEIGHT / 2,
                                                       dimensions[ 1 ] - ( SCROLLBAR_BUTTON_VISUAL_WIDTH + SCROLLBAR_HEIGHT ),
                                                       SCROLLBAR_HEIGHT / 2 ) ) // Inside side bottom button
                            {
                                place = IN_SIDE_BOTTOM_BUTTON;
                            }
                            else
                                place = IN_SIDE_BOTTOM_SPACE;
                        }
                }
                else
                    place = IN_CORNER_AREA;
            }
            
            break;
        case DROP:
        case PINCH:
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            break;
        default:
            throw exception( "scrollset::acceptEvent(): Unknown event type" );
            break;
        }
        
        // Handle events based on area /////////////////////////////////////////
        
        enum
        {
            NOTHING = 0,
            EVENT_USED = 1,
            SHOULD_ARRANGE = 2
        } final = NOTHING;
        
        if( e.type == STROKE
            && ( place != IN_NOWHERE || prev_place != IN_NOWHERE ) )            // We only care about stroke events here, scrolling is handled later
                                                                                // Also, given jade::group's implementation, this second check may be redundant
        {
            if( capturing )
            {
                if( e.stroke.dev_id == captured_dev )
                {
                    switch( capturing )
                    {
                    case NONE:
                        // Not possible
                        break;
                    case HORIZONTAL_BAR:
                        if( !( e.stroke.click & CLICK_PRIMARY ) )               // Released
                        {
                            slider_state[ 0 ] = UP;
                            capturing = NONE;
                            final = NOTHING;
                        }
                        else
                        {
                            dpi::points slide_space = dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) - slider_width[ 0 ];
                            
                            contents -> setScrollPercent( ( ( e.position[ 0 ] - capture_start[ 0 ] )
                                                            + ( capture_start[ 2 ] - SCROLLBAR_BUTTON_REAL_WIDTH ) )
                                                          / slide_space,
                                                          contents -> getScrollPercent().second );
                            
                            final = SHOULD_ARRANGE;
                        }
                        break;
                    case VERTICAL_BAR:
                        if( !( e.stroke.click & CLICK_PRIMARY ) )               // Released
                        {
                            slider_state[ 1 ] = UP;
                            capturing = NONE;
                            final = NOTHING;
                        }
                        else
                        {
                            dpi::points slide_space = dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) - slider_width[ 1 ];
                            
                            contents -> setScrollPercent( contents -> getScrollPercent().first,
                                                          ( e.position[ 1 ] - capture_start[ 1 ]
                                                            + capture_start[ 2 ] - SCROLLBAR_BUTTON_REAL_WIDTH )
                                                          / slide_space );
                            
                            final = SHOULD_ARRANGE;
                        }
                        break;
                    case LEFT_BUTTON:
                        if( !( place & IN_BOTTOM_LEFT_BUTTON )
                            && prev_place & IN_BOTTOM_LEFT_BUTTON )
                        {
                            horz_state[ 0 ] = UP;
                            capturing = NONE;
                            final = SHOULD_ARRANGE;
                        }
                        else
                            if( place & IN_BOTTOM_LEFT_BUTTON
                                && !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                contents -> scrollPoints( getSetting_num( "jb_ScrollDistance" ) * -1, 0 );
                                horz_state[ 0 ] = UP;
                                capturing = NONE;
                                final = SHOULD_ARRANGE;
                            }
                            else
                                final = EVENT_USED;
                        break;
                    case RIGHT_BUTTON:
                        if( !( place & IN_BOTTOM_RIGHT_BUTTON )
                            && prev_place & IN_BOTTOM_RIGHT_BUTTON )
                        {
                            horz_state[ 0 ] = UP;
                            capturing = NONE;
                            final = SHOULD_ARRANGE;
                        }
                        else
                            if( place & IN_BOTTOM_RIGHT_BUTTON
                                && !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                contents -> scrollPoints( getSetting_num( "jb_ScrollDistance" ), 0 );
                                horz_state[ 0 ] = UP;
                                capturing = NONE;
                                final = SHOULD_ARRANGE;
                            }
                            else
                                final = EVENT_USED;
                        break;
                    case TOP_BUTTON:
                        if( !( place & IN_SIDE_TOP_BUTTON )
                            && prev_place & IN_SIDE_TOP_BUTTON )
                        {
                            horz_state[ 0 ] = UP;
                            capturing = NONE;
                            final = SHOULD_ARRANGE;
                        }
                        else
                            if( place & IN_SIDE_TOP_BUTTON
                                && !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                contents -> scrollPoints( 0, getSetting_num( "jb_ScrollDistance" ) * -1 );
                                horz_state[ 0 ] = UP;
                                capturing = NONE;
                                final = SHOULD_ARRANGE;
                            }
                            else
                                final = EVENT_USED;
                        break;
                    case BOTTOM_BUTTON:
                        if( !( place & IN_SIDE_BOTTOM_BUTTON )
                            && prev_place & IN_SIDE_BOTTOM_BUTTON )
                        {
                            horz_state[ 0 ] = UP;
                            capturing = NONE;
                            final = SHOULD_ARRANGE;
                        }
                        else
                            if( place & IN_SIDE_BOTTOM_BUTTON
                                && !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                contents -> scrollPoints( 0, getSetting_num( "jb_ScrollDistance" ) );
                                horz_state[ 0 ] = UP;
                                capturing = NONE;
                                final = SHOULD_ARRANGE;
                            }
                            else
                                final = EVENT_USED;
                        break;
                    case CORNER:
                        if( !( place & IN_CORNER_AREA )
                            && prev_place & IN_CORNER_AREA )                    // Moved out of area
                        {
                            if( corner_state == DOWN )
                                corner_state = UP;
                            else
                                corner_state = EVIL;
                            
                            capturing = NONE;
                            
                            final = SHOULD_ARRANGE;
                        }
                        else
                            if( place & IN_CORNER_AREA
                                && !( e.stroke.click & CLICK_PRIMARY ) )        // Click release = trigger
                            {
                                if( corner_state == DOWN )
                                    corner_state = EVIL;
                                else
                                    corner_state = UP;
                                
                                capturing = NONE;
                                
                                final = SHOULD_ARRANGE;
                            }
                            else
                                final = EVENT_USED;
                        break;
                    default:
                        throw exception( "scrollset::acceptEvent(): Unknown capturing state" );
                    }
                    
                    if( !capturing )
                        deassociateDevice( captured_dev );
                }
                // else pass on to contents
            }
            
            if( !capturing )                                                    // Explicitly check else condition, as above may alter capturing state
            {
                if( place & IN_CONTROL_AREA && e.stroke.click & CLICK_PRIMARY )
                {
                    switch( place )
                    {
                    case IN_CORNER_AREA:
                        if( corner_state == EVIL )
                            corner_state = EVIL_DOWN;
                        else
                            corner_state = DOWN;
                        capturing = CORNER;
                        break;
                    case IN_BOTTOM_LEFT_BUTTON:
                        ff::write( jb_out, "Left button clicked\n" );
                        horz_state[ 0 ] = DOWN;
                        capturing = LEFT_BUTTON;
                        break;
                    case IN_BOTTOM_LEFT_SPACE:
                        // TODO: Click-to-jump
                        break;
                    case IN_BOTTOM_BAR:
                        slider_state[ 0 ] = DOWN;
                        capturing = HORIZONTAL_BAR;
                        capture_start[ 2 ] = slider_pos[ 0 ];
                        break;
                    case IN_BOTTOM_RIGHT_SPACE:
                        // TODO: Click-to-jump
                        break;
                    case IN_BOTTOM_RIGHT_BUTTON:
                        horz_state[ 1 ] = DOWN;
                        capturing = RIGHT_BUTTON;
                        break;
                    case IN_SIDE_TOP_BUTTON:
                        vert_state[ 0 ] = DOWN;
                        capturing = TOP_BUTTON;
                        break;
                    case IN_SIDE_TOP_SPACE:
                        // TODO: Click-to-jump
                        break;
                    case IN_SIDE_BAR:
                        slider_state[ 1 ] = DOWN;
                        capturing = VERTICAL_BAR;
                        capture_start[ 2 ] = slider_pos[ 1 ];
                        break;
                    case IN_SIDE_BOTTOM_SPACE:
                        // TODO: Click-to-jump
                        break;
                    case IN_SIDE_BOTTOM_BUTTON:
                        vert_state[ 1 ] = DOWN;
                        capturing = BOTTOM_BUTTON;
                        break;
                    default:
                        // All other cases handled in above sanity check
                        break;
                    }
                    
                    if( capturing )
                    {
                        captured_dev = e.stroke.dev_id;
                        associateDevice( captured_dev );
                        
                        capture_start[ 0 ] = e.position[ 0 ];
                        capture_start[ 1 ] = e.position[ 1 ];
                        
                        final = SHOULD_ARRANGE;
                    }
                    else if( final != SHOULD_ARRANGE )
                    {
                        final = EVENT_USED;
                    }
                }
                // else we don't care, so pass on to contents
            }
        }
        
        // Pass event to contents //////////////////////////////////////////////
        
        bool contents_accepted = false;
        if( place & IN_CONTENTS )                                               // Due to reasons, this and event_used can not both be true or false
            contents_accepted = contents -> acceptEvent( e );                   // The contents have a 0,0 relative position to the scrollset
        
        // Check for scrolling around contents /////////////////////////////////
        
        if( e.type == SCROLL && !contents_accepted )
        {
            if( ( place & IN_CORNER_AREA )
                || !( place & IN_ANYWHERE ) )
            {
                return false;                                                   // No scrolling in corner
            }
            
            contents -> scrollPoints( e.scroll.amount[ 0 ], e.scroll.amount[ 1 ] );
            
            arrangeBars();                                                      // May call parent -> requestRedraw()
            
            return true;
        }
        
        if( final == SHOULD_ARRANGE )
            arrangeBars();
        
        return final > NONE || contents_accepted;
    }
    
    void scrollset::draw( window* w )
    {
        scoped_lock< mutex > slock_e( element_mutex );
        scoped_lock< mutex > slock_r( scroll_rsrc_mutex );
        
        // Draw bars first (below) /////////////////////////////////////////////
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            for( int i = 0; i < 2; ++i )                                        // Iterate through the 2 scrollbars for code reuse
            {
                glPushMatrix();
                {
                    if( i == 1 )
                        glRotatef( 180.0f, 1.0f, 1.0f, 0.0f );                  // Flips around a vector pointing down left for the vertical bar
                                                                                // This means that the corners furthest from CORNER are always the draw origin
                    
                    if( slider_state[ i ] != DISABLED || bars_always_visible )
                    {
                        glTranslatef( 0.0f, dimensions[ 1 - i ] - SCROLLBAR_HEIGHT, 0.0f );
                        
                        glPushMatrix();
                        {
                            glScalef( dimensions[ i ], 1.0f, 1.0f );
                            scroll_set.fill -> draw( w );
                        }
                        glPopMatrix();
                    }
                    
                    if( slider_state[ i ] != DISABLED )
                    {
                        glPushMatrix();
                        {
                            for( int j = 0; j < 2; ++j )
                            {
                                switch( ( i ? horz_state : vert_state )[ j ] )  // Nice little bit of pointer magic
                                {
                                case UP:
                                    if( j )
                                        scroll_set.right_bottom.up -> draw( w );
                                    else
                                        scroll_set.left_top.up -> draw( w );
                                    break;
                                case DOWN:
                                    if( j )
                                        scroll_set.right_bottom.down -> draw( w );
                                    else
                                        scroll_set.left_top.down -> draw( w );
                                    break;
                                default:
                                    throw exception( "scrollset::draw(): Invalid/unknown button state" );
                                }
                                
                                if( !j )
                                {
                                    if( bars_always_visible
                                        || slider_state[ 1 - i ] != DISABLED )  // If the other slider takes up its space
                                    {
                                        glTranslatef( dimensions[ i ] - SCROLLBAR_BUTTON_VISUAL_WIDTH - SCROLLBAR_HEIGHT, 0.0f, 0.0f );
                                    }
                                    else
                                        glTranslatef( dimensions[ i ] - SCROLLBAR_BUTTON_VISUAL_WIDTH, 0.0f, 0.0f );
                                }
                            }
                        }
                        glPopMatrix();
                        
                        glTranslatef( slider_pos[ i ], 0.0f, 0.0f );
                        
                        #ifdef DEPRESSABLE_SLIDER_BARS
                        
                        switch( slider_state[ i ] )
                        {
                            case UP:
                                scroll_set.bar.left_top.up -> draw( w );
                                break;
                            case DOWN:
                                scroll_set.bar.left_top.down -> draw( w );
                                break;
                            default:
                                throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                        }
                        
                        #else
                        
                        scroll_set.bar.left_top.up -> draw( w );                // Always draw the bars "up"
                        
                        #endif
                        
                        glTranslatef( SLIDER_END_WIDTH, 0.0f, 0.0f );
                        
                        glPushMatrix();
                        {
                            glScalef( slider_width[ i ] - SLIDER_END_WIDTH * 2, 1.0f, 1.0f );
                            
                            #ifdef DEPRESSABLE_SLIDER_BARS
                            
                            switch( slider_state[ i ] )
                            {
                                case UP:
                                    scroll_set.bar.fill.up -> draw( w );
                                    break;
                                case DOWN:
                                    scroll_set.bar.fill.down -> draw( w );
                                    break;
                                default:
                                    throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                            }
                            
                            #else
                            
                            scroll_set.bar.fill.up -> draw( w );                // Always draw the bars "up," for now
                            
                            #endif
                        }
                        glPopMatrix();
                        
                        glTranslatef( slider_width[ i ] - SLIDER_END_WIDTH * 2, 0.0f, 0.0f );
                        
                        #ifdef DEPRESSABLE_SLIDER_BARS
                        
                        switch( slider_state[ i ] )
                        {
                            case UP:
                                scroll_set.bar.right_bottom.up -> draw( w );
                                break;
                            case DOWN:
                                scroll_set.bar.right_bottom.down -> draw( w );
                                break;
                            default:
                                throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                        }
                        
                        #else
                        
                        scroll_set.bar.right_bottom.up -> draw( w );
                        
                        #endif
                    }
                }
                glPopMatrix();
            }
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
        
        // Then draw corner ////////////////////////////////////////////////////
        
        if( bars_always_visible
            || slider_state[ 0 ] != DISABLED 
            || slider_state[ 1 ] != DISABLED )
        {
            glPushMatrix();
            {
                glTranslatef( dimensions[ 0 ] - SCROLLBAR_HEIGHT, dimensions[ 1 ] - SCROLLBAR_HEIGHT, 0.0f );
                
                switch( corner_state )
                {
                case UP:
                    scroll_set.corner.up -> draw( w );
                    break;
                case DOWN:
                case EVIL_DOWN:
                    scroll_set.corner.down -> draw( w );
                    break;
                case EVIL:
                    scroll_set.corner.evil -> draw( w );
                    break;
                default:
                    throw exception( "scrollset::draw(): Invalid corner state" );
                }
            }
            glPopMatrix();
        }
        
        // Finally draw contents (on top); they mask themselves ////////////////
        
        contents -> draw( w );                                                  // Contents keep track of their own position & dimensions
    }
    
    void scrollset::clearDeviceAssociations()                                   // clearDeviceAssociations() is not required to be thread-safe
    {
        switch( capturing )
        {
        case NONE:
            break;
        case HORIZONTAL_BAR:
        case VERTICAL_BAR:
        case LEFT_BUTTON:
        case RIGHT_BUTTON:
        case TOP_BUTTON:
        case BOTTOM_BUTTON:
        case CORNER:
            deassociateDevice( captured_dev );
            break;
        default:
            throw exception( "scrollset::clearDeviceAssociations(): Unknown capturing state" );
        }
    }
    
    void scrollset::arrangeBars()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::pair< dpi::points, dpi::points > scroll_limits = contents -> getScrollLimitPoints();
        std::pair< dpi::points, dpi::points > scroll_offsets = contents -> getScrollPoints();
        
        // Enable/disable slider bars, corner //////////////////////////////////
        
        if( scroll_limits.first == 0 )                                          // Horizontal
        {
            horz_state[ 0 ] = DISABLED;
            horz_state[ 1 ] = DISABLED;
            slider_state[ 0 ] = DISABLED;
        }
        else
        {
            horz_state[ 0 ] = UP;
            horz_state[ 1 ] = UP;
            slider_state[ 0 ] = UP;
        }
        
        if( scroll_limits.second == 0 )                                         // Vertical
        {
            vert_state[ 0 ] = DISABLED;
            vert_state[ 1 ] = DISABLED;
            slider_state[ 1 ] = DISABLED;
        }
        else
        {
            vert_state[ 0 ] = UP;
            vert_state[ 1 ] = UP;
            slider_state[ 1 ] = UP;
        }
        
        if( bars_always_visible
            || slider_state[ 0 ] != DISABLED
            || slider_state[ 1 ] != DISABLED )                                  // Corner
        {
            if( corner_state == DISABLED )
                corner_state = UP;
            // else leave in current state
        }
        else
            corner_state = DISABLED;
        
        // Set actual bar states ///////////////////////////////////////////////
        
        bool release_capture = false;
        
        switch( capturing )
        {
            case NONE:
                break;                                                          // Nothing needs to be done
            case HORIZONTAL_BAR:
                if( slider_state[ 0 ] == DISABLED )
                    release_capture = true;
                else
                    slider_state[ 0 ] = DOWN;
                break;
            case VERTICAL_BAR:
                if( slider_state[ 1 ] == DISABLED )
                    release_capture = true;
                else
                    slider_state[ 1 ] = DOWN;
                break;
            case LEFT_BUTTON:
                if( vert_state[ 0 ] == DISABLED )
                    release_capture = true;
                else
                    vert_state[ 0 ] = DOWN;
                break;
            case RIGHT_BUTTON:
                if( vert_state[ 1 ] == DISABLED )
                    release_capture = true;
                else
                    vert_state[ 1 ] = DOWN;
                break;
            case TOP_BUTTON:
                if( horz_state[ 0 ] == DISABLED )
                    release_capture = true;
                else
                    horz_state[ 0 ] = DOWN;
                break;
            case BOTTOM_BUTTON:
                if( horz_state[ 1 ] == DISABLED )
                    release_capture = true;
                else
                    horz_state[ 1 ] = DOWN;
                break;
            case CORNER:
                if( corner_state == DISABLED )
                    release_capture = true;
                // No else, acceptEvent() takes care of non-DISABLED state
                break;
            default:
                throw exception( "scrollset::arrangeBars(): Unknown capturing state" );
        }
        
        if( release_capture )
        {
            if( parent == NULL )
                throw exception( "scrollset::arrangeBars(): NULL parent for capture release" );
            
            deassociateDevice( captured_dev );
            capturing = NONE;
        }
        
        // Position slider bars ////////////////////////////////////////////////
        
        for( int i = 0; i < 2; ++i )
        {
            dpi::points scroll_limit = i ? scroll_limits.second : scroll_limits.first;
            dpi::points scroll_offset = i ? scroll_offsets.second : scroll_offsets.first;
            
            dpi::points visible_dim;
            
            if( bars_always_visible
                || slider_state[ 1 - i ] != DISABLED )
            {
                visible_dim = dimensions[ i ] - SCROLLBAR_HEIGHT;
            }
            else
                visible_dim = dimensions[ i ];                                  // If the other bar is invisible use entire width
            
            dpi::points max_length = visible_dim - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 );
            
            slider_width[ i ] = max_length * visible_dim
                                / ( visible_dim + ( scroll_limit * ( scroll_limit < 0 ? -1 : 1 ) ) );
            
            slider_pos[ i ] = SCROLLBAR_BUTTON_REAL_WIDTH + ( max_length - slider_width[ i ] )
                     * ( scroll_limit < 0 ? ( 1 - ( scroll_offset / scroll_limit ) ) : ( scroll_offset / scroll_limit ) );
        }
        
        // Resize contents /////////////////////////////////////////////////////
        
        dpi::points internal_dims[ 2 ];
        
        if( slider_state[ 0 ] != DISABLED
            || bars_always_visible )
        {
            internal_dims[ 0 ] = dimensions[ 0 ] - SCROLLBAR_HEIGHT;
        }
        else
            internal_dims[ 0 ] = dimensions[ 0 ];
        
        if( slider_state[ 1 ] != DISABLED
            || bars_always_visible )
        {
            internal_dims[ 1 ] = dimensions[ 1 ] - SCROLLBAR_HEIGHT;
        }
        else
            internal_dims[ 1 ] = dimensions[ 1 ];
        
        contents -> setRealDimensions( internal_dims[ 0 ], internal_dims[ 1 ] );
        
        // Request redraw //////////////////////////////////////////////////////
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    void scrollset::init()
    {
        if( !contents )
            throw exception( "scrollset::init(): Contents empty shared_ptr" );
        
        horz_state[ 0 ] = DISABLED;
        horz_state[ 1 ] = DISABLED;
        vert_state[ 0 ] = DISABLED;
        vert_state[ 1 ] = DISABLED;
        corner_state    = UP;
        
        slider_state[ 0 ] = DISABLED;
        slider_state[ 1 ] = DISABLED;
        
        capturing = NONE;
        
        bars_always_visible = false;
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        
        arrangeBars();
        
        scoped_lock< mutex > slock( scroll_rsrc_mutex );
        
        if( !got_resources )
        {
            scroll_set.left_top.up   = getNamedResource( scrollbar_button_left_top_up );
            scroll_set.left_top.down = getNamedResource( scrollbar_button_left_top_down );
            
            scroll_set.right_bottom.up   = getNamedResource( scrollbar_button_right_bottom_up );
            scroll_set.right_bottom.down = getNamedResource( scrollbar_button_right_bottom_down );
            
            scroll_set.bar.left_top.up       = getNamedResource( scrollbar_bar_left_top_up );
            scroll_set.bar.left_top.down     = getNamedResource( scrollbar_bar_left_top_down );
            scroll_set.bar.fill.up           = getNamedResource( scrollbar_bar_center_up );
            scroll_set.bar.fill.down         = getNamedResource( scrollbar_bar_center_down );
            scroll_set.bar.right_bottom.up   = getNamedResource( scrollbar_bar_right_bottom_up );
            scroll_set.bar.right_bottom.down = getNamedResource( scrollbar_bar_right_bottom_down );
            
            scroll_set.fill = getNamedResource( scrollbar_fill );
            
            scroll_set.corner.up   = getNamedResource( scrollbar_corner_up );
            scroll_set.corner.down = getNamedResource( scrollbar_corner_down );
            scroll_set.corner.evil = getNamedResource( scrollbar_corner_evil );
            
            got_resources = true;
        }
    }
}


