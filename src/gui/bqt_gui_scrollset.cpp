/* 
 * bqt_gui_scrollset.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_scrollset.hpp"

#include "bqt_gui_resource.hpp"
#include "bqt_gui_group.hpp"
#include "bqt_named_resources.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_preferences.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex scroll_rsrc_mutex;
    bool got_resources = false;
    
    struct button_set
    {
        bqt::gui_resource* up;
        bqt::gui_resource* down;
    };
    
    struct bar_set
    {
        button_set left_top;
        button_set fill;
        button_set right_bottom;
    };
    
    struct corner_set
    {
        bqt::gui_resource* up;
        bqt::gui_resource* down;
        bqt::gui_resource* evil;
    };
    
    struct
    {
        button_set left_top;
        button_set right_bottom;
        
        bar_set bar;
        
        bqt::gui_resource* fill;
        
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

namespace bqt
{
    void scrollset::arrangeBars()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::pair< int, int > scroll_limits = contents -> getScrollLimitPixels();
        std::pair< int, int > scroll_offsets = contents -> getScrollPixels();
        
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
            parent.deassociateDevice( captured_dev );
            capturing = NONE;
        }
        
        // Position slider bars ////////////////////////////////////////////////
        
        for( int i = 0; i < 2; ++i )
        {
            float scroll_limit = i ? scroll_limits.second : scroll_limits.first;
            float scroll_offset = i ? scroll_offsets.second : scroll_offsets.first;
            
            float visible_dim;
            
            if( bars_always_visible
                || slider_state[ 1 - i ] != DISABLED )
            {
                visible_dim = dimensions[ i ] - SCROLLBAR_HEIGHT;
            }
            else
                visible_dim = dimensions[ i ];                                  // If the other bar is invisible use entire width
            
            float max_length = visible_dim - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 );
            
            slider_width[ i ] = max_length * visible_dim
                                / ( visible_dim + ( scroll_limit * ( scroll_limit < 0 ? -1 : 1 ) ) );
            
            slider_pos[ i ] = SCROLLBAR_BUTTON_REAL_WIDTH + ( max_length - slider_width[ i ] )
                     * ( scroll_limit < 0 ? ( 1 - ( scroll_offset / scroll_limit ) ) : ( scroll_offset / scroll_limit ) );
        }
        
        parent.requestRedraw();
        
        // limit_percent contents_limits = contents -> getScrollLimitPercent();
        
        // int slide_space[ 2 ] = { ( dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) ),
        //                          ( dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_HEIGHT ) ) };
        
        // slider_width[ 0 ] = ( 1.0f / ( 1.0f + contents_limits.first.second - contents_limits.first.first ) )
        //                     * slide_space[ 0 ];
        // slider_width[ 1 ] = ( 1.0f / ( 1.0f + contents_limits.second.second - contents_limits.second.first ) )
        //                     * slide_space[ 1 ];
        
        // std::pair< float, float > contents_scroll = contents -> getScrollPercent();
        
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
    
    scrollset::scrollset( window& parent,
                          int x,
                          int y,
                          unsigned int w,
                          unsigned int h,
                          scrollable* c ) : gui_element( parent, x, y, w, h )
    {
        horz_state[ 0 ] = DISABLED;
        horz_state[ 1 ] = DISABLED;
        vert_state[ 0 ] = DISABLED;
        vert_state[ 1 ] = DISABLED;
        corner_state    = UP;
        
        slider_state[ 0 ] = DISABLED;
        slider_state[ 1 ] = DISABLED;
        
        capturing = NONE;
        
        bars_always_visible = false;
        
        if( c == NULL )
        {
            group* g = new group( parent,
                                  position[ 0 ],
                                  position[ 1 ],
                                  dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                  dimensions[ 1 ] - SCROLLBAR_HEIGHT );
            g -> setEventFallthrough( true );
            
            contents = g;
        }
        else
        {
            c -> setRealPosition( position[ 0 ], position[ 1 ] );
            c -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                    dimensions[ 1 ] - SCROLLBAR_HEIGHT );
            
            contents = c;
        }
        
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
    scrollset::~scrollset()
    {
        delete contents;
    }
    
    void scrollset::setRealPosition( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT );
        
        parent.requestRedraw();
    }
    void scrollset::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        ff::write( bqt_out,
                   "Setting scrollset dimensions to ",
                   w,
                   " x ",
                   h,
                   "\n" );
        
        contents -> setRealPosition( position[ 0 ], position[ 1 ] );
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_HEIGHT,
                                       dimensions[ 1 ] - SCROLLBAR_HEIGHT );
        
        arrangeBars();                                                          // Calls parent.requestRedraw()
    }
    
    void scrollset::setBarsAlwaysVisible( bool v )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        bars_always_visible = v;
        
        arrangeBars();                                                          // Calls parent.requestRedraw()
    }
    bool scrollset::getBarsAlwaysVisible()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return bars_always_visible;
    }
    
    bool scrollset::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( capturing
            && e.type == STROKE
            && e.stroke.dev_id != captured_dev )                                // Ignore other devices wile capturing another (just pass to contents)
        {
            return contents -> acceptEvent( e );
        }
        
        if( capturing && e.type == STROKE )
        {
            int rect_pos[ 2 ];
            int rect_dim[ 2 ];
            
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
                //         parent.deassociateDevice( e.stroke.dev_id );
                //         capturing = NONE;
                //     }
                //     arrangeBars();
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
                //         parent.deassociateDevice( e.stroke.dev_id );
                //         capturing = NONE;
                //     }
                //     arrangeBars();
                // }
                // else
                {
                    int scroll_amount[ 2 ];
                    
                    switch( capturing )
                    {
                        case LEFT_BUTTON:
                            scroll_amount[ 0 ] = getWheelScrollDistance();
                            scroll_amount[ 1 ] = 0;
                            break;
                        case RIGHT_BUTTON:
                            scroll_amount[ 0 ] = getWheelScrollDistance() * -1;
                            scroll_amount[ 1 ] = 0;
                            break;
                        case TOP_BUTTON:
                            scroll_amount[ 0 ] = 0;
                            scroll_amount[ 1 ] = getWheelScrollDistance();
                            break;
                        case BOTTOM_BUTTON:
                            scroll_amount[ 0 ] = 0;
                            scroll_amount[ 1 ] = getWheelScrollDistance() * -1;
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
                                contents -> scrollPixels( scroll_amount[ 0 ], scroll_amount[ 1 ] );
                            
                            parent.deassociateDevice( e.stroke.dev_id );
                            capturing = NONE;
                        }
                    }
                    else                                                        // Click cancel
                    {
                        parent.deassociateDevice( e.stroke.dev_id );
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
                    parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
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
            
            contents -> scrollPixels( e.scroll.amount[ 0 ], e.scroll.amount[ 1 ] );
            
            arrangeBars();                                                      // Calls parent.requestRedraw()
            
            return true;
        }
        
        return false;
    }
    
    void scrollset::draw()
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
                            scroll_set.fill -> draw();
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
                                        scroll_set.right_bottom.up -> draw();
                                    else
                                        scroll_set.left_top.up -> draw();
                                    break;
                                case DOWN:
                                    if( j )
                                        scroll_set.right_bottom.down -> draw();
                                    else
                                        scroll_set.left_top.down -> draw();
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
                                scroll_set.bar.left_top.up -> draw();
                                break;
                            case DOWN:
                                scroll_set.bar.left_top.down -> draw();
                                break;
                            default:
                                throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                        }
                        
                        #else
                        
                        scroll_set.bar.left_top.up -> draw();                   // Always draw the bars "up"
                        
                        #endif
                        
                        glTranslatef( SLIDER_END_WIDTH, 0.0f, 0.0f );
                        
                        glPushMatrix();
                        {
                            glScalef( slider_width[ i ] - SLIDER_END_WIDTH * 2, 1.0f, 1.0f );
                            
                            #ifdef DEPRESSABLE_SLIDER_BARS
                            
                            switch( slider_state[ i ] )
                            {
                                case UP:
                                    scroll_set.bar.fill.up -> draw();
                                    break;
                                case DOWN:
                                    scroll_set.bar.fill.down -> draw();
                                    break;
                                default:
                                    throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                            }
                            
                            #else
                            
                            scroll_set.bar.fill.up -> draw();                   // Always draw the bars "up," for now
                            
                            #endif
                        }
                        glPopMatrix();
                        
                        glTranslatef( slider_width[ i ] - SLIDER_END_WIDTH * 2, 0.0f, 0.0f );
                        
                        #ifdef DEPRESSABLE_SLIDER_BARS
                        
                        switch( slider_state[ i ] )
                        {
                            case UP:
                                scroll_set.bar.right_bottom.up -> draw();
                                break;
                            case DOWN:
                                scroll_set.bar.right_bottom.down -> draw();
                                break;
                            default:
                                throw exception( "scrollset::draw(): Invalid/unknown slider state" );
                        }
                        
                        #else
                        
                        scroll_set.bar.right_bottom.up -> draw();
                        
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
                    scroll_set.corner.up -> draw();
                    break;
                case DOWN:
                case EVIL_DOWN:
                    scroll_set.corner.down -> draw();
                    break;
                case EVIL:
                    scroll_set.corner.evil -> draw();
                    break;
                default:
                    throw exception( "scrollset::draw(): Invalid corner state" );
                }
            }
            glPopMatrix();
        }
        
        // Finally draw contents (on top); they mask themselves ////////////////
        
        contents -> draw();                                                     // Contents keep track of their own position & dimensions
    }
}


