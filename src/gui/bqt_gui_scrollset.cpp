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

#define SCROLLBAR_WIDTH                 12
#define MIN_SCROLLBAR_LENGTH            12
#define SCROLLBAR_BUTTON_REAL_WIDTH     18
#define SCROLLBAR_BUTTON_VISUAL_WIDTH   25
#define SLIDER_END_WIDTH                6

// #define DEPRESSABLE_SLIDER_BARS

namespace bqt
{
    void scrollset::arrangeBars()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        limit_percent contents_limits = contents -> getScrollLimitPercent();
        
        int slide_space[ 2 ] = { ( dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_WIDTH ) ),
                                 ( dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_WIDTH ) ) };
        
        slider_width[ 0 ] = ( 1.0f / ( 1.0f + contents_limits.first.second - contents_limits.first.first ) )
                            * slide_space[ 0 ];
        slider_width[ 1 ] = ( 1.0f / ( 1.0f + contents_limits.second.second - contents_limits.second.first ) )
                            * slide_space[ 1 ];
        
        std::pair< float, float > contents_scroll = contents -> getScrollPercent();
        
        slider_pos[ 0 ] = slider_width[ 0 ] - ( slide_space[ 0 ] - slider_width[ 0 ] ) * ( contents_scroll.first - contents_limits.first.first );
        slider_pos[ 1 ] = slider_width[ 1 ] - ( slide_space[ 1 ] - slider_width[ 1 ] ) * ( contents_scroll.second - contents_limits.second.first );
        
        if( capturing == LEFT_BUTTON )
            horz_state[ 0 ] = DOWN;
        else
            horz_state[ 0 ] = UP;
        if( capturing == RIGHT_BUTTON )
            horz_state[ 1 ] = DOWN;
        else
            horz_state[ 1 ] = UP;
        if( capturing == TOP_BUTTON )
            vert_state[ 0 ] = DOWN;
        else
            vert_state[ 0 ] = UP;
        if( capturing == BOTTOM_BUTTON )
            vert_state[ 1 ] = DOWN;
        else
            vert_state[ 1 ] = UP;
        
        if( slider_width[ 0 ] > slide_space[ 0 ] )
        {
            slider_state[ 0 ] = DISABLED;
            horz_state[ 0 ] = DISABLED;
            horz_state[ 1 ] = DISABLED;
        }
        else
        {
            if( capturing == HORIZONTAL_BAR )
                slider_state[ 0 ] = DOWN;
            else
                slider_state[ 0 ] = UP;
        }
        
        if( slider_width[ 1 ] > slide_space[ 1 ] )
        {
            slider_state[ 1 ] = DISABLED;
            vert_state[ 0 ] = DISABLED;
            vert_state[ 1 ] = DISABLED;
        }
        else
        {
            if( capturing == VERTICAL_BAR )
                slider_state[ 1 ] = DOWN;
            else
                slider_state[ 1 ] = UP;
        }
        
        parent.requestRedraw();
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
        
        if( c == NULL )
        {
            group* g = new group( parent,
                                  position[ 0 ],
                                  position[ 1 ],
                                  dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                  dimensions[ 1 ] - SCROLLBAR_WIDTH );
            g -> setEventFallthrough( true );
            
            contents = g;
        }
        else
        {
            c -> setRealPosition( position[ 0 ], position[ 1 ] );
            c -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                    dimensions[ 1 ] - SCROLLBAR_WIDTH );
            
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
            scroll_set.bar.fill.up         = getNamedResource( scrollbar_bar_center_up );
            scroll_set.bar.fill.down       = getNamedResource( scrollbar_bar_center_down );
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
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                       dimensions[ 1 ] - SCROLLBAR_WIDTH );
        
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
        contents -> setRealDimensions( dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                       dimensions[ 1 ] - SCROLLBAR_WIDTH );
        
        arrangeBars();                                                          // Calls parent.requestRedraw()
    }
    
    bool scrollset::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
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
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_WIDTH;
                    rect_dim[ 0 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_WIDTH;
                    break;
                case RIGHT_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_WIDTH - SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_WIDTH;
                    rect_dim[ 0 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_WIDTH;
                    break;
                case TOP_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_WIDTH;
                    rect_pos[ 1 ] = 0;
                    rect_dim[ 0 ] = SCROLLBAR_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    break;
                case BOTTOM_BUTTON:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_WIDTH;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_WIDTH - SCROLLBAR_BUTTON_REAL_WIDTH;
                    rect_dim[ 0 ] = SCROLLBAR_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_BUTTON_REAL_WIDTH;
                    break;
                case CORNER:
                    rect_pos[ 0 ] = dimensions[ 0 ] - SCROLLBAR_WIDTH;
                    rect_pos[ 1 ] = dimensions[ 1 ] - SCROLLBAR_WIDTH;
                    rect_dim[ 0 ] = SCROLLBAR_WIDTH;
                    rect_dim[ 1 ] = SCROLLBAR_WIDTH;
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
                
                // float slide_space[ 2 ] =  { ( float )( dimensions[ 0 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_WIDTH ) - slider_width[ 0 ] ),
                //                             ( float )( dimensions[ 1 ] - ( SCROLLBAR_BUTTON_REAL_WIDTH * 2 + SCROLLBAR_WIDTH ) - slider_width[ 1 ] ) };
                
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
                                            position[ 1 ] + dimensions[ 1 ] - SCROLLBAR_WIDTH,
                                            dimensions[ 0 ],
                                            SCROLLBAR_WIDTH );
        bool inside_vert = pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                            e.stroke.position[ 1 ] - e.offset[ 1 ],
                                            position[ 0 ] + dimensions[ 0 ] - SCROLLBAR_WIDTH,
                                            position[ 1 ],
                                            SCROLLBAR_WIDTH,
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
                        >= position[ 0 ] + dimensions[ 0 ] - SCROLLBAR_BUTTON_REAL_WIDTH - SCROLLBAR_WIDTH )
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
                        >= position[ 1 ] + dimensions[ 1 ] - SCROLLBAR_BUTTON_REAL_WIDTH - SCROLLBAR_WIDTH )
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
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            for( int i = 0; i < 2; ++i )                                        // This is some nasty stuff
            {
                glPushMatrix();
                {
                    if( i == 1 )
                        glRotatef( 180.0f, 1.0f, 1.0f, 0.0f );                  // Flips around a vector pointing down left
                    
                    glTranslatef( 0.0f, dimensions[ 1 - i ] - SCROLLBAR_WIDTH, 0.0f );
                    
                    glPushMatrix();
                    {
                        glScalef( dimensions[ i ], 1.0f, 1.0f );
                        scroll_set.fill -> draw();
                    }
                    glPopMatrix();
                    
                    if( slider_state[ i ] != DISABLED )                         // If the slider is disabled, so are the buttons
                    {
                        glPushMatrix();
                        {
                            for( int j = 0; j < 2; ++j )                        // More magic
                            {
                                switch( ( i ? vert_state : horz_state )[ j ] )  // And more
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
                                
                                if( !j )                                        // Not really neccessary, but probably faster
                                    glTranslatef( dimensions[ i ] - SCROLLBAR_BUTTON_VISUAL_WIDTH - SCROLLBAR_WIDTH, 0.0f, 0.0f );
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
        
        glPushMatrix();                                                         // Corner
        {
            glTranslatef( dimensions[ 0 ] - SCROLLBAR_WIDTH, dimensions[ 1 ] - SCROLLBAR_WIDTH, 0.0f );
            
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
        
        contents -> draw();                                                     // Contents keep track of their own position & dimensions
    }
}


