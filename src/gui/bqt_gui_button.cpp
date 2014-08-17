/* 
 * bqt_gui_button.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_button.hpp"

#include "../threading/bqt_mutex.hpp"
#include "../bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex button_rsrc_mutex;
    bool button_rsrc_init = false;
    
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
    
    shape_set round_set;
    shape_set square_set;
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
        
        scoped_lock< mutex > slock( button_rsrc_mutex );
        
        if( !button_rsrc_init )
        {
            round_set.off_up.top_left      = parent.getNamedResource( rounded_button_off_up_top_left );
            round_set.off_up.top_center    = parent.getNamedResource( rounded_button_off_up_top_center );
            round_set.off_up.top_right     = parent.getNamedResource( rounded_button_off_up_top_right );
            round_set.off_up.center_left   = parent.getNamedResource( rounded_button_off_up_center_left );
            round_set.off_up.center_center = parent.getNamedResource( rounded_button_off_up_center_center );
            round_set.off_up.center_right  = parent.getNamedResource( rounded_button_off_up_center_right );
            round_set.off_up.bottom_left   = parent.getNamedResource( rounded_button_off_up_bottom_left );
            round_set.off_up.bottom_center = parent.getNamedResource( rounded_button_off_up_bottom_center );
            round_set.off_up.bottom_right  = parent.getNamedResource( rounded_button_off_up_bottom_right );
            
            // round_set.off_down.top_left      = parent.getNamedResource(  );
            // round_set.off_down.top_center    = parent.getNamedResource(  );
            // round_set.off_down.top_right     = parent.getNamedResource(  );
            // round_set.off_down.center_left   = parent.getNamedResource(  );
            // round_set.off_down.center_center = parent.getNamedResource(  );
            // round_set.off_down.center_right  = parent.getNamedResource(  );
            // round_set.off_down.bottom_left   = parent.getNamedResource(  );
            // round_set.off_down.bottom_center = parent.getNamedResource(  );
            // round_set.off_down.bottom_right  = parent.getNamedResource(  );
            
            // round_set.on_up.top_left      = parent.getNamedResource(  );
            // round_set.on_up.top_center    = parent.getNamedResource(  );
            // round_set.on_up.top_right     = parent.getNamedResource(  );
            // round_set.on_up.center_left   = parent.getNamedResource(  );
            // round_set.on_up.center_center = parent.getNamedResource(  );
            // round_set.on_up.center_right  = parent.getNamedResource(  );
            // round_set.on_up.bottom_left   = parent.getNamedResource(  );
            // round_set.on_up.bottom_center = parent.getNamedResource(  );
            // round_set.on_up.bottom_right  = parent.getNamedResource(  );
            
            // round_set.on_down.top_left      = parent.getNamedResource(  );
            // round_set.on_down.top_center    = parent.getNamedResource(  );
            // round_set.on_down.top_right     = parent.getNamedResource(  );
            // round_set.on_down.center_left   = parent.getNamedResource(  );
            // round_set.on_down.center_center = parent.getNamedResource(  );
            // round_set.on_down.center_right  = parent.getNamedResource(  );
            // round_set.on_down.bottom_left   = parent.getNamedResource(  );
            // round_set.on_down.bottom_center = parent.getNamedResource(  );
            // round_set.on_down.bottom_right  = parent.getNamedResource(  );
            
            // square_set.off_up.top_left      = parent.getNamedResource(  );
            // square_set.off_up.top_center    = parent.getNamedResource(  );
            // square_set.off_up.top_right     = parent.getNamedResource(  );
            // square_set.off_up.center_left   = parent.getNamedResource(  );
            // square_set.off_up.center_center = parent.getNamedResource(  );
            // square_set.off_up.center_right  = parent.getNamedResource(  );
            // square_set.off_up.bottom_left   = parent.getNamedResource(  );
            // square_set.off_up.bottom_center = parent.getNamedResource(  );
            // square_set.off_up.bottom_right  = parent.getNamedResource(  );
            
            // square_set.off_down.top_left      = parent.getNamedResource(  );
            // square_set.off_down.top_center    = parent.getNamedResource(  );
            // square_set.off_down.top_right     = parent.getNamedResource(  );
            // square_set.off_down.center_left   = parent.getNamedResource(  );
            // square_set.off_down.center_center = parent.getNamedResource(  );
            // square_set.off_down.center_right  = parent.getNamedResource(  );
            // square_set.off_down.bottom_left   = parent.getNamedResource(  );
            // square_set.off_down.bottom_center = parent.getNamedResource(  );
            // square_set.off_down.bottom_right  = parent.getNamedResource(  );
            
            // square_set.on_up.top_left      = parent.getNamedResource(  );
            // square_set.on_up.top_center    = parent.getNamedResource(  );
            // square_set.on_up.top_right     = parent.getNamedResource(  );
            // square_set.on_up.center_left   = parent.getNamedResource(  );
            // square_set.on_up.center_center = parent.getNamedResource(  );
            // square_set.on_up.center_right  = parent.getNamedResource(  );
            // square_set.on_up.bottom_left   = parent.getNamedResource(  );
            // square_set.on_up.bottom_center = parent.getNamedResource(  );
            // square_set.on_up.bottom_right  = parent.getNamedResource(  );
            
            // square_set.on_down.top_left      = parent.getNamedResource(  );
            // square_set.on_down.top_center    = parent.getNamedResource(  );
            // square_set.on_down.top_right     = parent.getNamedResource(  );
            // square_set.on_down.center_left   = parent.getNamedResource(  );
            // square_set.on_down.center_center = parent.getNamedResource(  );
            // square_set.on_down.center_right  = parent.getNamedResource(  );
            // square_set.on_down.bottom_left   = parent.getNamedResource(  );
            // square_set.on_down.bottom_center = parent.getNamedResource(  );
            // square_set.on_down.bottom_right  = parent.getNamedResource(  );
            
            button_rsrc_init = true;
        }
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
        ff::write( bqt_out, "Button recieved an event\n" );
        
        return false;
    }
    
    void button::draw()
    {
        scoped_lock< rwlock > sclock( button_lock, RW_READ );
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        
        {
            glPushMatrix(); // Draw vertical centers
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, 1.0f, 1.0f );
                round_set.off_up.top_center -> draw();
                glTranslatef( 0.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                round_set.off_up.bottom_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Draw horizontal centers
            {
                glTranslatef( 0.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( 1.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                round_set.off_up.center_left -> draw();
                glTranslatef( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                round_set.off_up.center_right -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Center
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                round_set.off_up.center_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix(); // Corners
            {
                round_set.off_up.top_left -> draw();
                glTranslatef( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                round_set.off_up.top_right -> draw();
                glTranslatef( 0.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                round_set.off_up.bottom_right -> draw();
                glTranslatef( ( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f ) * -1.0f, 0.0f, 0.0f );
                round_set.off_up.bottom_left -> draw();
            }
            glPopMatrix();
        }
        
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


