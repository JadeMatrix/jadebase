/* 
 * bqt_gui_button.cpp
 * 
 * Implements GUI buttons
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_button.hpp"

#include "../bqt_mutex.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_log.hpp"
#include "bqt_gui_resource.hpp"
#include "bqt_gui_resource_names.hpp"

/******************************************************************************//******************************************************************************/

#define BUTTON_MIN_WIDTH    12
#define BUTTON_MIN_HEIGHT   14

namespace bqt
{
    button::button( layout& parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : layout_element( parent, x, y, w, h )
    {
        type = BLANK;
        auto_resize = false;
        togglable = false;
        state = OFF_UP;
        
        {
            off_up.top_left = parent.getNamedResource( rounded_button_off_up_top_left );
            off_up.top_center = parent.getNamedResource( rounded_button_off_up_top_center );
            off_up.top_right = parent.getNamedResource( rounded_button_off_up_top_right );
            off_up.center_left = parent.getNamedResource( rounded_button_off_up_center_left );
            off_up.center_center = parent.getNamedResource( rounded_button_off_up_center_center );
            off_up.center_right = parent.getNamedResource( rounded_button_off_up_center_right );
            off_up.bottom_left = parent.getNamedResource( rounded_button_off_up_bottom_left );
            off_up.bottom_center = parent.getNamedResource( rounded_button_off_up_bottom_center );
            off_up.bottom_right = parent.getNamedResource( rounded_button_off_up_bottom_right );

            off_down.top_left = parent.getNamedResource( rounded_button_off_down_top_left );
            off_down.top_center = parent.getNamedResource( rounded_button_off_down_top_center );
            off_down.top_right = parent.getNamedResource( rounded_button_off_down_top_right );
            off_down.center_left = parent.getNamedResource( rounded_button_off_down_center_left );
            off_down.center_center = parent.getNamedResource( rounded_button_off_down_center_center );
            off_down.center_right = parent.getNamedResource( rounded_button_off_down_center_right );
            off_down.bottom_left = parent.getNamedResource( rounded_button_off_down_bottom_left );
            off_down.bottom_center = parent.getNamedResource( rounded_button_off_down_bottom_center );
            off_down.bottom_right = parent.getNamedResource( rounded_button_off_down_bottom_right );

            on_up.top_left = parent.getNamedResource( rounded_button_on_up_top_left );
            on_up.top_center = parent.getNamedResource( rounded_button_on_up_top_center );
            on_up.top_right = parent.getNamedResource( rounded_button_on_up_top_right );
            on_up.center_left = parent.getNamedResource( rounded_button_on_up_center_left );
            on_up.center_center = parent.getNamedResource( rounded_button_on_up_center_center );
            on_up.center_right = parent.getNamedResource( rounded_button_on_up_center_right );
            on_up.bottom_left = parent.getNamedResource( rounded_button_on_up_bottom_left );
            on_up.bottom_center = parent.getNamedResource( rounded_button_on_up_bottom_center );
            on_up.bottom_right = parent.getNamedResource( rounded_button_on_up_bottom_right );

            on_down.top_left = parent.getNamedResource( rounded_button_on_down_top_left );
            on_down.top_center = parent.getNamedResource( rounded_button_on_down_top_center );
            on_down.top_right = parent.getNamedResource( rounded_button_on_down_top_right );
            on_down.center_left = parent.getNamedResource( rounded_button_on_down_center_left );
            on_down.center_center = parent.getNamedResource( rounded_button_on_down_center_center );
            on_down.center_right = parent.getNamedResource( rounded_button_on_down_center_right );
            on_down.bottom_left = parent.getNamedResource( rounded_button_on_down_bottom_left );
            on_down.bottom_center = parent.getNamedResource( rounded_button_on_down_bottom_center );
            on_down.bottom_right = parent.getNamedResource( rounded_button_on_down_bottom_right );
        }
    }
    button::~button()
    {
        
    }
    
    std::pair< unsigned int, unsigned int > button::getRealDimensions()
    {
        scoped_lock slock( element_mutex );
        
        if( auto_resize )
        {
            // TODO: get contents' dimensions
        }
        
        if( dimensions[ 0 ] < BUTTON_MIN_WIDTH )
            dimensions[ 0 ] = BUTTON_MIN_WIDTH;
        
        if( dimensions[ 1 ] < BUTTON_MIN_HEIGHT )
            dimensions[ 1 ] = BUTTON_MIN_HEIGHT;
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    // std::pair< unsigned int, unsigned int > button::getVisualDimensions()
    // {
    //     return getRealDimensions();
    // }
    
    bool button::acceptEvent( window_event&e )
    {
        scoped_lock slock( element_mutex );
        
        return false;
    }
    
    void button::draw()
    {
        scoped_lock slock( element_mutex );
        
        std::pair< unsigned int, unsigned int > real_dimensions = getRealDimensions();
        
        ff::write( bqt_out,
                   "Drawing a ",
                   real_dimensions.first,
                   "x",
                   real_dimensions.second,
                   " button\n" );
        
        rsrc_group* group;
        
        switch( state )
        {
        case OFF_UP:
            group = &off_up;
            break;
        case OFF_DOWN:
            group = &off_down;
            break;
        case ON_UP:
            group = &on_up;
            break;
        case ON_DOWN:
            group = &on_down;
            break;
        default:
            throw exception( "button::draw(): Unknown state" );
        }
        
        glPushMatrix();                                                         // Draw vertical centers
        {
            glTranslatef( BUTTON_MIN_WIDTH / 2, 0.0, 0.0 );
            
            glScalef( real_dimensions.first - BUTTON_MIN_WIDTH, 1.0, 1.0 );
            
            group -> top_center -> draw();
            
            glTranslatef( 0.0, real_dimensions.second - BUTTON_MIN_HEIGHT / 2, 0.0 );
            
            group -> bottom_center -> draw();
        }
        glPopMatrix();
        
        glPushMatrix();                                                         // Draw horizontal centers
        {
            glTranslatef( 0.0, BUTTON_MIN_HEIGHT / 2, 0.0 );
            
            glScalef( 1.0, real_dimensions.second - BUTTON_MIN_HEIGHT, 1.0 );
            
            group -> center_left -> draw();
            
            glTranslatef( real_dimensions.first - BUTTON_MIN_WIDTH / 2, 0.0, 0.0 );
            
            group -> center_right -> draw();
        }
        glPopMatrix();
        
        glPushMatrix();                                                         // Center
        {
            glTranslatef( BUTTON_MIN_WIDTH / 2, BUTTON_MIN_HEIGHT / 2, 0.0 );
            
            glScalef( real_dimensions.first - BUTTON_MIN_WIDTH, real_dimensions.second - BUTTON_MIN_HEIGHT, 1.0 );
            
            group -> center_center -> draw();
        }
        glPopMatrix();
        
        glPushMatrix();                                                         // Corners
        {
            group -> top_left -> draw();
            
            glTranslatef( real_dimensions.first - BUTTON_MIN_WIDTH / 2, 0.0, 0.0 );
            
            group -> top_right -> draw();
            
            glTranslatef( 0.0, real_dimensions.second - BUTTON_MIN_HEIGHT / 2, 0.0 );
            
            group -> bottom_right -> draw();
            
            glTranslatef( -1 * ( real_dimensions.first - BUTTON_MIN_WIDTH / 2 ), 0.0, 0.0 );
            
            group -> bottom_left -> draw();
        }
        glPopMatrix();
        
        // TODO: render contents
    }
    
    /* button-specific ****************************************************//******************************************************************************/
    
    void button::setText ( std::string t, bool c )
    {
        scoped_lock slock( element_mutex );
        
        str_contents = t;
        
        if( c )
            type = STRING;
    }
    // void button::setImage( ???? i, bool );
    void button::setImage( std::string f, bool c )
    {
        scoped_lock slock( element_mutex );
        
        throw exception( "button::setImage( std::string, bool ): Not implemented yet" );
        // TODO: implement
        
        if( c )
            type = IMAGE;
    }
    
    void button::clearContents( bool c )
    {
        scoped_lock slock( element_mutex );
        
        str_contents = "";
        // img_contents
        
        if( c )
            type = BLANK;
    }
    
    void button::setAutoResize( bool a )
    {
        scoped_lock slock( element_mutex );
        
        auto_resize = a;
    }
    bool button::getAutoResize()
    {
        scoped_lock slock( element_mutex );
        
        return auto_resize;
    }
    
    void button::setTogglable( bool t )
    {
        scoped_lock slock( element_mutex );
        
        togglable = t;
    }
    bool button::getTogglable()
    {
        scoped_lock slock( element_mutex );
        
        return togglable;
    }
}


