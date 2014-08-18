/* 
 * bqt_gui_button.cpp
 * 
 * Implements bqt_gui_button.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_button.hpp"

#include <map>

#include "../bqt_log.hpp"
#include "../bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct state_set
    {
        bqt::gui_resource* top_left;
        bqt::gui_resource* top_center;
        bqt::gui_resource* top_right;
        bqt::gui_resource* center_left;
        bqt::gui_resource* center_center;
        bqt::gui_resource* center_right;
        bqt::gui_resource* bottom_left;
        bqt::gui_resource* bottom_center;
        bqt::gui_resource* bottom_right;
    };
    
    struct shape_set
    {
        state_set off_up;
        state_set off_down;
        state_set on_up;
        state_set on_down;
    };
    
    struct button_set
    {
        shape_set rounded_set;
        shape_set squared_set;
    };
    
    bqt::rwlock button_rsrc_lock;
    std::map< bqt::window*, button_set > button_sets;
    std::map< bqt::window*, int > button_set_counts;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    button::button( window& parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : gui_element( parent, x, y, w, h )
    {
        state = OFF_UP;
        
        scoped_lock< rwlock > slock( button_rsrc_lock, RW_WRITE );
        
        if( !button_sets.count( &parent ) )
        {
            button_sets[ &parent ].rounded_set.off_up.top_left      = parent.getNamedResource( rounded_button_off_up_top_left );
            button_sets[ &parent ].rounded_set.off_up.top_center    = parent.getNamedResource( rounded_button_off_up_top_center );
            button_sets[ &parent ].rounded_set.off_up.top_right     = parent.getNamedResource( rounded_button_off_up_top_right );
            button_sets[ &parent ].rounded_set.off_up.center_left   = parent.getNamedResource( rounded_button_off_up_center_left );
            button_sets[ &parent ].rounded_set.off_up.center_center = parent.getNamedResource( rounded_button_off_up_center_center );
            button_sets[ &parent ].rounded_set.off_up.center_right  = parent.getNamedResource( rounded_button_off_up_center_right );
            button_sets[ &parent ].rounded_set.off_up.bottom_left   = parent.getNamedResource( rounded_button_off_up_bottom_left );
            button_sets[ &parent ].rounded_set.off_up.bottom_center = parent.getNamedResource( rounded_button_off_up_bottom_center );
            button_sets[ &parent ].rounded_set.off_up.bottom_right  = parent.getNamedResource( rounded_button_off_up_bottom_right );

            button_sets[ &parent ].rounded_set.off_down.top_left      = parent.getNamedResource( rounded_button_off_down_top_left );
            button_sets[ &parent ].rounded_set.off_down.top_center    = parent.getNamedResource( rounded_button_off_down_top_center );
            button_sets[ &parent ].rounded_set.off_down.top_right     = parent.getNamedResource( rounded_button_off_down_top_right );
            button_sets[ &parent ].rounded_set.off_down.center_left   = parent.getNamedResource( rounded_button_off_down_center_left );
            button_sets[ &parent ].rounded_set.off_down.center_center = parent.getNamedResource( rounded_button_off_down_center_center );
            button_sets[ &parent ].rounded_set.off_down.center_right  = parent.getNamedResource( rounded_button_off_down_center_right );
            button_sets[ &parent ].rounded_set.off_down.bottom_left   = parent.getNamedResource( rounded_button_off_down_bottom_left );
            button_sets[ &parent ].rounded_set.off_down.bottom_center = parent.getNamedResource( rounded_button_off_down_bottom_center );
            button_sets[ &parent ].rounded_set.off_down.bottom_right  = parent.getNamedResource( rounded_button_off_down_bottom_right );

            button_sets[ &parent ].rounded_set.on_up.top_left      = parent.getNamedResource( rounded_button_on_up_top_left );
            button_sets[ &parent ].rounded_set.on_up.top_center    = parent.getNamedResource( rounded_button_on_up_top_center );
            button_sets[ &parent ].rounded_set.on_up.top_right     = parent.getNamedResource( rounded_button_on_up_top_right );
            button_sets[ &parent ].rounded_set.on_up.center_left   = parent.getNamedResource( rounded_button_on_up_center_left );
            button_sets[ &parent ].rounded_set.on_up.center_center = parent.getNamedResource( rounded_button_on_up_center_center );
            button_sets[ &parent ].rounded_set.on_up.center_right  = parent.getNamedResource( rounded_button_on_up_center_right );
            button_sets[ &parent ].rounded_set.on_up.bottom_left   = parent.getNamedResource( rounded_button_on_up_bottom_left );
            button_sets[ &parent ].rounded_set.on_up.bottom_center = parent.getNamedResource( rounded_button_on_up_bottom_center );
            button_sets[ &parent ].rounded_set.on_up.bottom_right  = parent.getNamedResource( rounded_button_on_up_bottom_right );

            button_sets[ &parent ].rounded_set.on_down.top_left      = parent.getNamedResource( rounded_button_on_down_top_left );
            button_sets[ &parent ].rounded_set.on_down.top_center    = parent.getNamedResource( rounded_button_on_down_top_center );
            button_sets[ &parent ].rounded_set.on_down.top_right     = parent.getNamedResource( rounded_button_on_down_top_right );
            button_sets[ &parent ].rounded_set.on_down.center_left   = parent.getNamedResource( rounded_button_on_down_center_left );
            button_sets[ &parent ].rounded_set.on_down.center_center = parent.getNamedResource( rounded_button_on_down_center_center );
            button_sets[ &parent ].rounded_set.on_down.center_right  = parent.getNamedResource( rounded_button_on_down_center_right );
            button_sets[ &parent ].rounded_set.on_down.bottom_left   = parent.getNamedResource( rounded_button_on_down_bottom_left );
            button_sets[ &parent ].rounded_set.on_down.bottom_center = parent.getNamedResource( rounded_button_on_down_bottom_center );
            button_sets[ &parent ].rounded_set.on_down.bottom_right  = parent.getNamedResource( rounded_button_on_down_bottom_right );

            button_sets[ &parent ].squared_set.off_up.top_left      = parent.getNamedResource( squared_button_off_up_top_left );
            button_sets[ &parent ].squared_set.off_up.top_center    = parent.getNamedResource( squared_button_off_up_top_center );
            button_sets[ &parent ].squared_set.off_up.top_right     = parent.getNamedResource( squared_button_off_up_top_right );
            button_sets[ &parent ].squared_set.off_up.center_left   = parent.getNamedResource( squared_button_off_up_center_left );
            button_sets[ &parent ].squared_set.off_up.center_center = parent.getNamedResource( squared_button_off_up_center_center );
            button_sets[ &parent ].squared_set.off_up.center_right  = parent.getNamedResource( squared_button_off_up_center_right );
            button_sets[ &parent ].squared_set.off_up.bottom_left   = parent.getNamedResource( squared_button_off_up_bottom_left );
            button_sets[ &parent ].squared_set.off_up.bottom_center = parent.getNamedResource( squared_button_off_up_bottom_center );
            button_sets[ &parent ].squared_set.off_up.bottom_right  = parent.getNamedResource( squared_button_off_up_bottom_right );

            button_sets[ &parent ].squared_set.off_down.top_left      = parent.getNamedResource( squared_button_off_down_top_left );
            button_sets[ &parent ].squared_set.off_down.top_center    = parent.getNamedResource( squared_button_off_down_top_center );
            button_sets[ &parent ].squared_set.off_down.top_right     = parent.getNamedResource( squared_button_off_down_top_right );
            button_sets[ &parent ].squared_set.off_down.center_left   = parent.getNamedResource( squared_button_off_down_center_left );
            button_sets[ &parent ].squared_set.off_down.center_center = parent.getNamedResource( squared_button_off_down_center_center );
            button_sets[ &parent ].squared_set.off_down.center_right  = parent.getNamedResource( squared_button_off_down_center_right );
            button_sets[ &parent ].squared_set.off_down.bottom_left   = parent.getNamedResource( squared_button_off_down_bottom_left );
            button_sets[ &parent ].squared_set.off_down.bottom_center = parent.getNamedResource( squared_button_off_down_bottom_center );
            button_sets[ &parent ].squared_set.off_down.bottom_right  = parent.getNamedResource( squared_button_off_down_bottom_right );

            button_sets[ &parent ].squared_set.on_up.top_left      = parent.getNamedResource( squared_button_on_up_top_left );
            button_sets[ &parent ].squared_set.on_up.top_center    = parent.getNamedResource( squared_button_on_up_top_center );
            button_sets[ &parent ].squared_set.on_up.top_right     = parent.getNamedResource( squared_button_on_up_top_right );
            button_sets[ &parent ].squared_set.on_up.center_left   = parent.getNamedResource( squared_button_on_up_center_left );
            button_sets[ &parent ].squared_set.on_up.center_center = parent.getNamedResource( squared_button_on_up_center_center );
            button_sets[ &parent ].squared_set.on_up.center_right  = parent.getNamedResource( squared_button_on_up_center_right );
            button_sets[ &parent ].squared_set.on_up.bottom_left   = parent.getNamedResource( squared_button_on_up_bottom_left );
            button_sets[ &parent ].squared_set.on_up.bottom_center = parent.getNamedResource( squared_button_on_up_bottom_center );
            button_sets[ &parent ].squared_set.on_up.bottom_right  = parent.getNamedResource( squared_button_on_up_bottom_right );

            button_sets[ &parent ].squared_set.on_down.top_left      = parent.getNamedResource( squared_button_on_down_top_left );
            button_sets[ &parent ].squared_set.on_down.top_center    = parent.getNamedResource( squared_button_on_down_top_center );
            button_sets[ &parent ].squared_set.on_down.top_right     = parent.getNamedResource( squared_button_on_down_top_right );
            button_sets[ &parent ].squared_set.on_down.center_left   = parent.getNamedResource( squared_button_on_down_center_left );
            button_sets[ &parent ].squared_set.on_down.center_center = parent.getNamedResource( squared_button_on_down_center_center );
            button_sets[ &parent ].squared_set.on_down.center_right  = parent.getNamedResource( squared_button_on_down_center_right );
            button_sets[ &parent ].squared_set.on_down.bottom_left   = parent.getNamedResource( squared_button_on_down_bottom_left );
            button_sets[ &parent ].squared_set.on_down.bottom_center = parent.getNamedResource( squared_button_on_down_bottom_center );
            button_sets[ &parent ].squared_set.on_down.bottom_right  = parent.getNamedResource( squared_button_on_down_bottom_right );
            
            button_set_counts[ &parent ] = 1;
        }
        else
            button_set_counts[ &parent ]++;
    }
    button::~button()
    {
        scoped_lock< rwlock > slock( button_rsrc_lock, RW_WRITE );
        
        button_set_counts[ &parent ]--;
        
        if( button_set_counts[ &parent ] < 1 )
            button_set_counts.erase( &parent );
    }
    
    void button::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< rwlock > slock( button_lock, RW_WRITE );
        
        if( w < BUTTON_MIN_WIDTH )
            dimensions[ 0 ] = BUTTON_MIN_WIDTH;
        else
            dimensions[ 0 ] = w;
        
        if( h < BUTTON_MIN_HEIGHT )
            dimensions[ 1 ] = BUTTON_MIN_HEIGHT;
        else
            dimensions[ 1 ] = h;
    }
    
    bool button::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock_b( button_lock, RW_WRITE );
        
        switch( e.type )
        {
        // case STROKE:
        //     {
        //         if( e.stroke.pressure > 0.0f )
        //         {
                    
        //         }
        //     }
        case KEYCOMMAND:
            if( e.key.key == KEY_Space && !e.key.up )
            {
                switch( state )
                {
                case OFF_UP:
                    state = OFF_DOWN;
                    parent.requestRedraw();
                    break;
                case ON_UP:
                    state = ON_DOWN;
                    parent.requestRedraw();
                    break;
                case OFF_DOWN:
                case ON_DOWN:
                    // Ignore
                    break;
                default:
                    throw exception( "button::acceptEvent(): Unknown state" );
                }
            }
            if( e.key.key == KEY_Space && e.key.up )
            {
                switch( state )
                {
                case OFF_DOWN:
                    state = ON_UP;
                    parent.requestRedraw();
                    break;
                case ON_DOWN:
                    state = OFF_UP;
                    parent.requestRedraw();
                    break;
                case OFF_UP:
                case ON_UP:
                    // Ignore
                    break;
                default:
                    throw exception( "button::acceptEvent(): Unknown state" );
                }
            }
        default:
            return false;
        }
    }
    
    void button::draw()
    {
        scoped_lock< rwlock > slock_b( button_lock, RW_READ );
        scoped_lock< rwlock > slock_r( button_rsrc_lock, RW_READ );
        
        button_set& window_bset( button_sets[ &parent ] );
        
        state_set* top_set;
        state_set* center_set;
        state_set* bottom_set;
        
        switch( state )
        {
        case OFF_UP:
            top_set    = &window_bset.rounded_set.off_up;
            center_set = &window_bset.rounded_set.off_up;
            bottom_set = &window_bset.rounded_set.off_up;
            break;
        case OFF_DOWN:
            top_set    = &window_bset.rounded_set.off_down;
            center_set = &window_bset.rounded_set.off_down;
            bottom_set = &window_bset.rounded_set.off_down;
            break;
        case ON_UP:
            top_set    = &window_bset.rounded_set.on_up;
            center_set = &window_bset.rounded_set.on_up;
            bottom_set = &window_bset.rounded_set.on_up;
            break;
        case ON_DOWN:
            top_set    = &window_bset.rounded_set.on_down;
            center_set = &window_bset.rounded_set.on_down;
            bottom_set = &window_bset.rounded_set.on_down;
            break;
        default:
            throw exception( "button::draw(): Unknown state" );
        }
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        
        {
            glPushMatrix(); // Draw vertical centers
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, 1.0f, 1.0f );
                top_set -> top_center -> draw();
                glTranslatef( 0.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                bottom_set -> bottom_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Draw horizontal centers
            {
                glTranslatef( 0.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( 1.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                center_set -> center_left -> draw();
                glTranslatef( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                center_set -> center_right -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Center
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                center_set -> center_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Corners
            {
                top_set -> top_left -> draw();
                glTranslatef( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                top_set -> top_right -> draw();
                glTranslatef( 0.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                bottom_set -> bottom_right -> draw();
                glTranslatef( ( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f ) * -1.0f, 0.0f, 0.0f );
                bottom_set -> bottom_left -> draw();
            }
            glPopMatrix();
        }
        
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


