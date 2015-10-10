/* 
 * jb_scrollset.cpp
 * 
 * About
 * 
 */

// FIXME: Everything.
// FIXME: Fix this entire mess so it works

// offset_temp[ i ] = scroll_offset[ i ] + offset[ i ];

// if( offset_temp[ i ] / scroll_limits[ i ] >= 0 )  // same direction, clip to limit
// {
//     if( scroll_limits[ i ] < 0
//         ? offset_temp[ i ] < scroll_limits[ i ]
//         : offset_temp[ i ] > scroll_limits[ i ] )
//     {
//         offset[ i ] = scroll_limits[ i ] - scroll_offset[ i ];
//     }
// }
// else  // different directions, clip to origin
// {
//     offset[ i ] = scroll_offset[ i ] * -1;  // will set scroll_offset[ i ] to 0
// }






// visible_dims[ i ] = dimensions[ i ] - SCROLLBAR_HEIGHT;
// max_length[ i ] = visible_dims[ i ] - ( SCROLLBUTTON_LENGTH * 2 );

// scrollbar_length[ i ] = max_length[ i ] * visible_dims[ i ] / ( visible_dims[ i ] + ( scroll_limits[ i ]
//                                                                                       * ( scroll_limits[ i ] < 0 ? -1 : 1 ) ) );

// scrollbar_pos[ i ] = SCROLLBUTTON_LENGTH + ( max_length[ i ] - scrollbar_length[ i ] )
//                      * ( scroll_limit[ i ] < 0 ? ( 1 - ( offset[ i ] / scroll_limit[ i ] ) ) : ( offset[ i ] / scroll_limit[ i ] ) );

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

#define SCROLLBAR_HEIGHT                12
#define MIN_SCROLLBAR_LENGTH            12
#define SCROLLBAR_BUTTON_REAL_WIDTH     18                                      // Width of the button bounds
#define SCROLLBAR_BUTTON_VISUAL_WIDTH   25                                      // Width of the button sprite
#define SLIDER_END_WIDTH                6

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
        
        contents -> setRealDimensions( dimensions[ 0 ],
                                       dimensions[ 1 ] );
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
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT );
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void scrollset::setRealDimensions( dpi::points w, dpi::points h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        ff::write( jb_out,
                   "Setting scrollset dimensions to ",
                   w,
                   " x ",
                   h,
                   "\n" );
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
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
        
        if( parent == NULL )
            throw exception( "scrollset::acceptEvent(): NULL parent window" );
        
        if( capturing
            && e.type == STROKE
            && e.stroke.dev_id != captured_dev )                                // Ignore other devices wile capturing another (just pass to contents)
        {
            return contents -> acceptEvent( e );
        }
        
        if( capturing && e.type == STROKE )
        {
            dpi::points rect_pos[ 2 ];
            dpi::points rect_dim[ 2 ];
            
            switch( capturing )
            {
                case HORIZONTAL_BAR:
                case VERTICAL_BAR:                                              // Bars capture even outside
                    break;
                case LEFT_BUTTON:
                    rect_pos[ 0 ] = 0;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_HEIGHT;
                    rect_dim[ 0 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_HEIGHT;
                    break;
                case RIGHT_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_HEIGHT - SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_HEIGHT;
                    rect_dim[ 0 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_HEIGHT;
                    break;
                case TOP_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_HEIGHT;
                    rect_pos[ 1 ] = 0;
                    rect_dim[ 0 ] = SCROLLBAR_HEIGHT;
                    rect_dim[ 1 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    break;
                case BOTTOM_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_HEIGHT;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_HEIGHT - SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 0 ] = SCROLLBAR_HEIGHT;
                    rect_dim[ 1 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    break;
                case CORNER:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_HEIGHT;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_HEIGHT;
                    rect_dim[ 0 ] = SCROLLBAR_HEIGHT;
                    rect_dim[ 1 ] = SCROLLBAR_HEIGHT;
                    break;
                case NONE:
                    /* won't get here */
                default:
                    throw exception( "scrollset::acceptEvent(): Unknown capturing state" );
            }
            
            bool point_in_button;
            bool prev_in_button;
            
            if( capturing == HORIZONTAL_BAR
                || capturing == VERTICAL_BAR
                || ( point_in_button = pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                                        e.stroke.position[ 1 ] - e.offset[ 1 ],
                                                        rect_pos[ 0 ] + position[ 0 ],
                                                        rect_pos[ 1 ] + position[ 1 ],
                                                        rect_dim[ 0 ],
                                                        rect_dim[ 1 ] ) )
                || ( prev_in_button  = pointInsideRect( e.stroke.prev_pos[ 0 ] - e.offset[ 0 ],
                                                        e.stroke.prev_pos[ 1 ] - e.offset[ 1 ],
                                                        rect_pos[ 0 ] + position[ 0 ],
                                                        rect_pos[ 1 ] + position[ 1 ],
                                                        rect_dim[ 0 ],
                                                        rect_dim[ 1 ] ) ) )
            {
                // limit_percent contents_limits = contents -> getScrollLimitPercent();
                // std::pair< float, float > contents_scroll = contents -> getScrollPercent();
                
                // float slide_space[ 2 ] =  { ( float )( dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) - slider_width[ 0 ] ),
                //                             ( float )( dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) - slider_width[ 1 ] ) };
                
                // // ( ( e.stroke.position[ 0 ] - e.offset[ 0 ] - capture_start[ 0 ] - capture_start[ 2 ] )
                // // / slide_space[ 0 ] ) / ( 1.0f + contents_limits.first.second - contents_limits.first.first )
                
                // if( capturing == HORIZONTAL_BAR )
                // {
                //     if( e.stroke.click & CLICK_PRIMARY )
                //     {
                //         contents -> setScrollPercent( ( e.stroke.position[ 0 ] - e.offset[ 0 ] - capture_start[ 0 ] - capture_start[ 2 ] ) / slide_space[ 0 ],
                //                                       contents_scroll.second );
                //     }
                //     else
                //     {
                //         deassociateDevice( e.stroke.dev_id );
                //         capturing = NONE;
                //     }
                //     arrangeBars();
                //     return false;        // didn't use
                // }
                // else if( capturing == VERTICAL_BAR )
                // {
                //     if( e.stroke.click & CLICK_PRIMARY )
                //     {
                //         contents -> setScrollPercent( contents_scroll.first,
                //                                       ( e.stroke.position[ 1 ] - e.offset[ 1 ] - capture_start[ 1 ] - capture_start[ 2 ] ) / slide_space[ 1 ] );
                //     }
                //     else
                //     {
                //         deassociateDevice( e.stroke.dev_id );
                //         capturing = NONE;
                //     }
                //     arrangeBars();
                //     return false;        // didn't use
                // }
                // else
                {
                    int scroll_amount[ 2 ];
                    
                    switch( capturing )
                    {
                        case LEFT_BUTTON:
                            scroll_amount[ 0 ] = getSetting_num( "jb_ScrollDistance" );
                            scroll_amount[ 1 ] = 0;
                            break;
                        case RIGHT_BUTTON:
                            scroll_amount[ 0 ] = getSetting_num( "jb_ScrollDistance" ) * -1;
                            scroll_amount[ 1 ] = 0;
                            break;
                        case TOP_BUTTON:
                            scroll_amount[ 0 ] = 0;
                            scroll_amount[ 1 ] = getSetting_num( "jb_ScrollDistance" );
                            break;
                        case BOTTOM_BUTTON:
                            scroll_amount[ 0 ] = 0;
                            scroll_amount[ 1 ] = getSetting_num( "jb_ScrollDistance" ) * -1;
                            break;
                        case CORNER:
                            break;
                        default:
                            throw exception( "scrollset::acceptEvent(): Unknown/invalid capturing state" );
                    }
                    
                    if( point_in_button )
                    {
                        if( !( e.stroke.click & CLICK_PRIMARY ) )               // Successful click
                        {
                            // if( capturing == CORNER )
                            // {
                            //     if( corner_state == DOWN )
                            //         corner_state = EVIL;
                            //     else
                            //         corner_state = UP;
                            // }
                            // else
                                contents -> scrollPoints( scroll_amount[ 0 ], scroll_amount[ 1 ] );
                            
                            deassociateDevice( e.stroke.dev_id );
                            capturing = NONE;
                        }
                    }
                    else                                                        // Click cancel
                    {
                        deassociateDevice( e.stroke.dev_id );
                        capturing = NONE;
                    }
                    arrangeBars();
                }
                
                return true;
            }
        }
        
        bool inside_horz = pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                            e.stroke.position[ 1 ] - e.offset[ 1 ],
                                            position[ 0 ],
                                            position[ 1 ] + dimensions[ 1 ] - SCROLLBAR_HEIGHT,
                                            dimensions[ 0 ],
                                            SCROLLBAR_HEIGHT );
        bool inside_vert = pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                            e.stroke.position[ 1 ] - e.offset[ 1 ],
                                            position[ 0 ] + dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                            position[ 1 ],
                                            SCROLLBAR_HEIGHT,
                                            dimensions[ 1 ] );
        bool inside_corner = inside_horz && inside_vert;
        
        if( inside_corner )
        {
            inside_horz = false;
            inside_vert = false;
        }
        
        if( inside_horz || inside_vert || inside_corner )
        {
            if( e.type == STROKE )
            {
                if( inside_horz && ( e.stroke.click & CLICK_PRIMARY ) )
                {
                    // if( e.stroke.position[ 0 ] - e.offset[ 0 ] >= position[ 0 ] + slider_pos[ 0 ] + SCROLLBAR_BUTTON_REAL_WIDTH
                    //     && e.stroke.position[ 0 ] - e.offset[ 0 ] < position[ 0 ] + slider_pos[ 0 ] + SCROLLBAR_BUTTON_REAL_WIDTH + slider_width[ 0 ] )
                    // {
                    //     capture_start[ 2 ] = slider_pos[ 0 ];
                    //     capturing = HORIZONTAL_BAR;
                    // }
                    
                    if( e.stroke.position[ 0 ] - e.offset[ 0 ]
                        < position[ 0 ] + SCROLLBAR_BUTTON_REAL_WIDTH )
                    {
                        capturing = LEFT_BUTTON;
                    }
                    
                    if( e.stroke.position[ 0 ] - e.offset[ 0 ]
                        >= position[ 0 ] + dimensions[ 0 ] - SCROLLBAR_BUTTON_REAL_WIDTH - SCROLLBAR_HEIGHT )
                    {
                        capturing = RIGHT_BUTTON;
                    }
                }
                else if( inside_vert && ( e.stroke.click & CLICK_PRIMARY ) )
                {
                    // if( e.stroke.position[ 1 ] - e.offset[ 1 ] >= position[ 1 ] + slider_pos[ 1 ] + SCROLLBAR_BUTTON_REAL_WIDTH
                    //     && e.stroke.position[ 1 ] - e.offset[ 1 ] < position[ 1 ] + slider_pos[ 1 ] + SCROLLBAR_BUTTON_REAL_WIDTH + slider_width[ 1 ] )
                    // {
                    //     capture_start[ 2 ] = slider_pos[ 1 ];
                    //     capturing = VERTICAL_BAR;
                    // }
                    
                    if( e.stroke.position[ 1 ] - e.offset[ 1 ]
                        < position[ 1 ] + SCROLLBAR_BUTTON_REAL_WIDTH )
                    {
                        capturing = TOP_BUTTON;
                    }
                    
                    if( e.stroke.position[ 1 ] - e.offset[ 1 ]
                        >= position[ 1 ] + dimensions[ 1 ] - SCROLLBAR_BUTTON_REAL_WIDTH - SCROLLBAR_HEIGHT )
                    {
                        capturing = BOTTOM_BUTTON;
                    }
                }
                // else if( inside_corner && ( e.stroke.click & CLICK_PRIMARY ) )
                // {
                //     if( corner_state == UP )
                //         corner_state = DOWN;
                //     else
                //         corner_state = EVIL_DOWN;
                    
                //     capturing = CORNER;
                // }
                
                if( capturing )
                {
                    capture_start[ 0 ] = e.stroke.position[ 0 ] + e.offset[ 0 ];
                    capture_start[ 1 ] = e.stroke.position[ 1 ] + e.offset[ 1 ];
                    associateDevice( e.stroke.dev_id, e.offset[ 0 ], e.offset[ 1 ] );
                    captured_dev = e.stroke.dev_id;
                    
                    arrangeBars();
                }
                
                return true;
            }
        }
        
        bool contents_accepted = contents -> acceptEvent( e );
        
        if( e.type == SCROLL && !contents_accepted )
        {
            if( inside_corner )
                return false;                                                   // No scrolling in corner
            
            contents -> scrollPoints( e.scroll.amount[ 0 ], e.scroll.amount[ 1 ] );
            
            arrangeBars();                                                      // May call parent -> requestRedraw()
            
            return true;
        }
        
        return false;
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
                                switch( ( i ? vert_state : horz_state )[ j ] )  // Nice little bit of pointer magic
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
                        
                        glTranslatef( slider_pos[ i ] + SCROLLBAR_BUTTON_REAL_WIDTH, 0.0f, 0.0f );
                        
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
        
        if( parent != NULL )
            parent -> requestRedraw();
        
        // limit_percent contents_limits = contents -> getScrollLimitPercent();
        
        // dpi::points slide_space[ 2 ] = { ( dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) ),
        //                                 ( dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) ) };
        
        // slider_width[ 0 ] = ( 1.0f / ( 1.0f + contents_limits.first.second - contents_limits.first.first ) )
        //                     * slide_space[ 0 ];
        // slider_width[ 1 ] = ( 1.0f / ( 1.0f + contents_limits.second.second - contents_limits.second.first ) )
        //                     * slide_space[ 1 ];
        
        // std::pair< dpi::points, dpi::points > contents_scroll = contents -> getScrollPercent();
        
        // slider_pos[ 0 ] = slider_width[ 0 ] - ( slide_space[ 0 ] - slider_width[ 0 ] ) * ( contents_scroll.first - contents_limits.first.first );
        // slider_pos[ 1 ] = slider_width[ 1 ] - ( slide_space[ 1 ] - slider_width[ 1 ] ) * ( contents_scroll.second - contents_limits.second.first );
        
        // if( capturing == LEFT_BUTTON )
        //     horz_state[ 0 ] = DOWN;
        // else
        //     horz_state[ 0 ] = UP;
        // if( capturing == RIGHT_BUTTON )
        //     horz_state[ 1 ] = DOWN;
        // else
        //     horz_state[ 1 ] = UP;
        // if( capturing == TOP_BUTTON )
        //     vert_state[ 0 ] = DOWN;
        // else
        //     vert_state[ 0 ] = UP;
        // if( capturing == BOTTOM_BUTTON )
        //     vert_state[ 1 ] = DOWN;
        // else
        //     vert_state[ 1 ] = UP;
        
        // if( slider_width[ 0 ] > slide_space[ 0 ] )
        // {
        //     slider_state[ 0 ] = DISABLED;
        //     horz_state[ 0 ] = DISABLED;
        //     horz_state[ 1 ] = DISABLED;
        // }
        // else
        // {
        //     if( capturing == HORIZONTAL_BAR )
        //         slider_state[ 0 ] = DOWN;
        //     else
        //         slider_state[ 0 ] = UP;
        // }
        
        // if( slider_width[ 1 ] > slide_space[ 1 ] )
        // {
        //     slider_state[ 1 ] = DISABLED;
        //     vert_state[ 0 ] = DISABLED;
        //     vert_state[ 1 ] = DISABLED;
        // }
        // else
        // {
        //     if( capturing == VERTICAL_BAR )
        //         slider_state[ 1 ] = DOWN;
        //     else
        //         slider_state[ 1 ] = UP;
        // }
        
        // parent.requestRedraw();
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


