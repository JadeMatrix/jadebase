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
        button_set center;
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

#define SCROLLBAR_WIDTH 12

namespace bqt
{
    void scrollset::arrangeBars()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        
        
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
        
        capturing = false;
        
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
            scroll_set.bar.center.up         = getNamedResource( scrollbar_bar_center_up );
            scroll_set.bar.center.down       = getNamedResource( scrollbar_bar_center_down );
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
                if( inside_horz )
                {
                    
                }
                else if( inside_vert )
                {
                    
                }
                else if( inside_corner )
                {
                    switch( corner_state )
                    {
                    case UP:
                        if( inside_corner && e.stroke.click & CLICK_PRIMARY )
                        {
                            corner_state = DOWN;
                            parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                            parent.requestRedraw();
                            return true;
                        }
                        break;
                    case DOWN:
                        if( inside_corner )
                        {
                            if( !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                corner_state = EVIL;
                                parent.deassociateDevice( e.stroke.dev_id );
                                parent.requestRedraw();
                            }
                            return true;
                        }
                        else
                        {
                            corner_state = UP;
                            parent.deassociateDevice( e.stroke.dev_id );
                            parent.requestRedraw();
                            return true;
                        }
                        break;
                    case EVIL:
                        if( inside_corner && e.stroke.click & CLICK_PRIMARY )
                        {
                            corner_state = EVIL_DOWN;
                            parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                            parent.requestRedraw();
                            return true;
                        }
                        break;
                    case EVIL_DOWN:
                        if( inside_corner )
                        {
                            if( !( e.stroke.click & CLICK_PRIMARY ) )
                            {
                                corner_state = UP;
                                parent.deassociateDevice( e.stroke.dev_id );
                                parent.requestRedraw();
                            }
                            return true;
                        }
                        else
                        {
                            corner_state = EVIL;
                            parent.deassociateDevice( e.stroke.dev_id );
                            parent.requestRedraw();
                            return true;
                        }
                        break;
                    default:
                        throw exception( "scrollset::acceptEvent(): Unknown corner state" );
                    }
                }
            }
        }
        
        bool contents_accepted = contents -> acceptEvent( e );
        
        if( e.type == SCROLL && !contents_accepted )
        {
            if( inside_corner )
                return false;                                                   // No scrolling in corner
            
            contents -> scrollPixels( e.scroll.amount[ 0 ], e.scroll.amount[ 1 ] );
            
            return true;
        }
        
        return false;
    }
    
    void scrollset::draw()
    {
        scoped_lock< mutex > slock_e( element_mutex );
        scoped_lock< mutex > slock_r( scroll_rsrc_mutex );
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        
        glPushMatrix();                                                         // Horizontal bar
        {
            glTranslatef( 0.0f, dimensions[ 1 ] - SCROLLBAR_WIDTH, 0.0f );
            
            glScalef( dimensions[ 0 ], 1.0f, 1.0f );
            
            scroll_set.fill -> draw();
        }
        glPopMatrix();
        
        for( int i = 0; i < 4; ++i )                                            // Horizontal buttons
        {
            if( horz_state[ i ] != DISABLED )
            {
                glPushMatrix();
                {
                    switch( horz_state[ 0 ] )
                    {
                    case UP:
                        scroll_set.corner.up -> draw();
                        break;
                    case DOWN:
                        scroll_set.corner.down -> draw();
                        break;
                    default:
                        throw exception( "scrollset::draw(): Unknown button state" );
                    }
                }
                glPopMatrix();
            }
        }
        
        glPushMatrix();                                                         // Vertical bar
        {
            glTranslatef( dimensions[ 0 ] - SCROLLBAR_WIDTH, 0.0f, 0.0f );
            
            glRotatef( 180.0f, 1.0f, 1.0f, 0.0f );                              // flips around a vector pointing down left
            
            glScalef( dimensions[ 0 ], 1.0f, 1.0f );
            
            scroll_set.fill -> draw();
        }
        glPopMatrix();
        
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
        
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
        
        contents -> draw();                                                     // Contents keep track of their own position & dimensions
    }
}


