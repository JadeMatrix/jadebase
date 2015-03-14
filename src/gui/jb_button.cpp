/* 
 * jb_button.cpp
 * 
 * Implements jade::button GUI element class
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_button.hpp"

#include <cmath>
#include <map>

#include "jb_named_resources.hpp"
#include "jb_resource.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_log.hpp"
#include "../threading/jb_mutex.hpp"
#include "../windowsys/jb_window.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex button_rsrc_mutex;
    bool got_resources = false;
    
    struct state_set
    {
        jade::gui_resource* top_left;
        jade::gui_resource* top_center;
        jade::gui_resource* top_right;
        jade::gui_resource* center_left;
        jade::gui_resource* center_center;
        jade::gui_resource* center_right;
        jade::gui_resource* bottom_left;
        jade::gui_resource* bottom_center;
        jade::gui_resource* bottom_right;
    };
    
    struct shape_set
    {
        state_set off_up;
        state_set off_down;
        state_set on_up;
        state_set on_down;
    };
    
    struct
    {
        shape_set rounded_set;
        shape_set squared_set;
    } button_set;
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    button::button( window* parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : gui_element( parent, x, y, w, h )
    {
        state = OFF_UP;
        
        contents_align = CENTER_CENTER;
        
        scoped_lock< mutex > slock( button_rsrc_mutex );
        
        if( !got_resources )
        {
            button_set.rounded_set.off_up.top_left      = getNamedResource( rounded_button_off_up_top_left );
            button_set.rounded_set.off_up.top_center    = getNamedResource( rounded_button_off_up_top_center );
            button_set.rounded_set.off_up.top_right     = getNamedResource( rounded_button_off_up_top_right );
            button_set.rounded_set.off_up.center_left   = getNamedResource( rounded_button_off_up_center_left );
            button_set.rounded_set.off_up.center_center = getNamedResource( rounded_button_off_up_center_center );
            button_set.rounded_set.off_up.center_right  = getNamedResource( rounded_button_off_up_center_right );
            button_set.rounded_set.off_up.bottom_left   = getNamedResource( rounded_button_off_up_bottom_left );
            button_set.rounded_set.off_up.bottom_center = getNamedResource( rounded_button_off_up_bottom_center );
            button_set.rounded_set.off_up.bottom_right  = getNamedResource( rounded_button_off_up_bottom_right );

            button_set.rounded_set.off_down.top_left      = getNamedResource( rounded_button_off_down_top_left );
            button_set.rounded_set.off_down.top_center    = getNamedResource( rounded_button_off_down_top_center );
            button_set.rounded_set.off_down.top_right     = getNamedResource( rounded_button_off_down_top_right );
            button_set.rounded_set.off_down.center_left   = getNamedResource( rounded_button_off_down_center_left );
            button_set.rounded_set.off_down.center_center = getNamedResource( rounded_button_off_down_center_center );
            button_set.rounded_set.off_down.center_right  = getNamedResource( rounded_button_off_down_center_right );
            button_set.rounded_set.off_down.bottom_left   = getNamedResource( rounded_button_off_down_bottom_left );
            button_set.rounded_set.off_down.bottom_center = getNamedResource( rounded_button_off_down_bottom_center );
            button_set.rounded_set.off_down.bottom_right  = getNamedResource( rounded_button_off_down_bottom_right );

            button_set.rounded_set.on_up.top_left      = getNamedResource( rounded_button_on_up_top_left );
            button_set.rounded_set.on_up.top_center    = getNamedResource( rounded_button_on_up_top_center );
            button_set.rounded_set.on_up.top_right     = getNamedResource( rounded_button_on_up_top_right );
            button_set.rounded_set.on_up.center_left   = getNamedResource( rounded_button_on_up_center_left );
            button_set.rounded_set.on_up.center_center = getNamedResource( rounded_button_on_up_center_center );
            button_set.rounded_set.on_up.center_right  = getNamedResource( rounded_button_on_up_center_right );
            button_set.rounded_set.on_up.bottom_left   = getNamedResource( rounded_button_on_up_bottom_left );
            button_set.rounded_set.on_up.bottom_center = getNamedResource( rounded_button_on_up_bottom_center );
            button_set.rounded_set.on_up.bottom_right  = getNamedResource( rounded_button_on_up_bottom_right );

            button_set.rounded_set.on_down.top_left      = getNamedResource( rounded_button_on_down_top_left );
            button_set.rounded_set.on_down.top_center    = getNamedResource( rounded_button_on_down_top_center );
            button_set.rounded_set.on_down.top_right     = getNamedResource( rounded_button_on_down_top_right );
            button_set.rounded_set.on_down.center_left   = getNamedResource( rounded_button_on_down_center_left );
            button_set.rounded_set.on_down.center_center = getNamedResource( rounded_button_on_down_center_center );
            button_set.rounded_set.on_down.center_right  = getNamedResource( rounded_button_on_down_center_right );
            button_set.rounded_set.on_down.bottom_left   = getNamedResource( rounded_button_on_down_bottom_left );
            button_set.rounded_set.on_down.bottom_center = getNamedResource( rounded_button_on_down_bottom_center );
            button_set.rounded_set.on_down.bottom_right  = getNamedResource( rounded_button_on_down_bottom_right );

            button_set.squared_set.off_up.top_left      = getNamedResource( squared_button_off_up_top_left );
            button_set.squared_set.off_up.top_center    = getNamedResource( squared_button_off_up_top_center );
            button_set.squared_set.off_up.top_right     = getNamedResource( squared_button_off_up_top_right );
            button_set.squared_set.off_up.center_left   = getNamedResource( squared_button_off_up_center_left );
            button_set.squared_set.off_up.center_center = getNamedResource( squared_button_off_up_center_center );
            button_set.squared_set.off_up.center_right  = getNamedResource( squared_button_off_up_center_right );
            button_set.squared_set.off_up.bottom_left   = getNamedResource( squared_button_off_up_bottom_left );
            button_set.squared_set.off_up.bottom_center = getNamedResource( squared_button_off_up_bottom_center );
            button_set.squared_set.off_up.bottom_right  = getNamedResource( squared_button_off_up_bottom_right );

            button_set.squared_set.off_down.top_left      = getNamedResource( squared_button_off_down_top_left );
            button_set.squared_set.off_down.top_center    = getNamedResource( squared_button_off_down_top_center );
            button_set.squared_set.off_down.top_right     = getNamedResource( squared_button_off_down_top_right );
            button_set.squared_set.off_down.center_left   = getNamedResource( squared_button_off_down_center_left );
            button_set.squared_set.off_down.center_center = getNamedResource( squared_button_off_down_center_center );
            button_set.squared_set.off_down.center_right  = getNamedResource( squared_button_off_down_center_right );
            button_set.squared_set.off_down.bottom_left   = getNamedResource( squared_button_off_down_bottom_left );
            button_set.squared_set.off_down.bottom_center = getNamedResource( squared_button_off_down_bottom_center );
            button_set.squared_set.off_down.bottom_right  = getNamedResource( squared_button_off_down_bottom_right );

            button_set.squared_set.on_up.top_left      = getNamedResource( squared_button_on_up_top_left );
            button_set.squared_set.on_up.top_center    = getNamedResource( squared_button_on_up_top_center );
            button_set.squared_set.on_up.top_right     = getNamedResource( squared_button_on_up_top_right );
            button_set.squared_set.on_up.center_left   = getNamedResource( squared_button_on_up_center_left );
            button_set.squared_set.on_up.center_center = getNamedResource( squared_button_on_up_center_center );
            button_set.squared_set.on_up.center_right  = getNamedResource( squared_button_on_up_center_right );
            button_set.squared_set.on_up.bottom_left   = getNamedResource( squared_button_on_up_bottom_left );
            button_set.squared_set.on_up.bottom_center = getNamedResource( squared_button_on_up_bottom_center );
            button_set.squared_set.on_up.bottom_right  = getNamedResource( squared_button_on_up_bottom_right );

            button_set.squared_set.on_down.top_left      = getNamedResource( squared_button_on_down_top_left );
            button_set.squared_set.on_down.top_center    = getNamedResource( squared_button_on_down_top_center );
            button_set.squared_set.on_down.top_right     = getNamedResource( squared_button_on_down_top_right );
            button_set.squared_set.on_down.center_left   = getNamedResource( squared_button_on_down_center_left );
            button_set.squared_set.on_down.center_center = getNamedResource( squared_button_on_down_center_center );
            button_set.squared_set.on_down.center_right  = getNamedResource( squared_button_on_down_center_right );
            button_set.squared_set.on_down.bottom_left   = getNamedResource( squared_button_on_down_bottom_left );
            button_set.squared_set.on_down.bottom_center = getNamedResource( squared_button_on_down_bottom_center );
            button_set.squared_set.on_down.bottom_right  = getNamedResource( squared_button_on_down_bottom_right );
            
            got_resources = true;
        }
    }
    
    void button::setToggleOnCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        toggle_on_callback = cb;
    }
    void button::setToggleOffCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        toggle_off_callback = cb;
    }
    
    void button::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( w < BUTTON_MIN_WIDTH )
            dimensions[ 0 ] = BUTTON_MIN_WIDTH;
        else
            dimensions[ 0 ] = w;
        
        if( h < BUTTON_MIN_HEIGHT )
            dimensions[ 1 ] = BUTTON_MIN_HEIGHT;
        else
            dimensions[ 1 ] = h;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    // FIXME: Store resize flag so we are always resizing; contents' size may not be valid when adding
    void button::setContents( const std::shared_ptr< gui_resource >& c,
                              resource_align a,
                              bool r )
    {
        contents = c;
        contents_align = a;
        
        if( contents && r )
        {
            std::pair< unsigned int, unsigned int > rsrc_dim = contents -> getDimensions();
            
            setRealDimensions( rsrc_dim.first, rsrc_dim.second );
        }
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    bool button::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock_b( element_mutex );
        
        if( parent == NULL )
            throw exception( "button::acceptEvent(): NULL parent window" );
        
        switch( e.type )
        {
        case STROKE:
            // WARNING: Consider using a series of if statements here, as in
            // the very unlikely case that the event.stroke's position and
            // prev_pos are the same & both outside the button (window::
            // acceptEvent() should prevent this) as the button state will bug
            // out.
            
            if( ( state == OFF_DOWN
                  || state == ON_DOWN )
                && e.stroke.dev_id != captured_dev )                            // Ignore but accept other devices wile capturing another
            {
                return true;
            }
            
            switch( state )
            {
            case OFF_UP:
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                        e.stroke.position[ 1 ] - e.offset[ 1 ],
                                        position[ 0 ],
                                        position[ 1 ],
                                        dimensions[ 0 ],
                                        dimensions[ 1 ] ) )
                {
                    setState( OFF_DOWN );
                    parent -> associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                    captured_dev = e.stroke.dev_id;
                }
                break;
            case OFF_DOWN:
                if( pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                     e.stroke.position[ 1 ] - e.offset[ 1 ],
                                     position[ 0 ],
                                     position[ 1 ],
                                     dimensions[ 0 ],
                                     dimensions[ 1 ] ) )
                {
                    if( !( e.stroke.click & CLICK_PRIMARY ) )
                    {
                        setState( ON_UP );
                        parent -> deassociateDevice( e.stroke.dev_id );
                    }
                }
                else                                                            // Works because we still get strokes that have just gone out of the mask
                {
                    setState( OFF_UP );                                         // Cancel the button press
                    parent -> deassociateDevice( e.stroke.dev_id );
                    return false;                                               // Stroke went out of the button
                }
                break;
            case ON_UP:
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                        e.stroke.position[ 1 ] - e.offset[ 1 ],
                                        position[ 0 ],
                                        position[ 1 ],
                                        dimensions[ 0 ],
                                        dimensions[ 1 ] ) )
                {
                    setState( ON_DOWN );
                    parent -> associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                    captured_dev = e.stroke.dev_id;
                }
                break;
            case ON_DOWN:
                if( pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                     e.stroke.position[ 1 ] - e.offset[ 1 ],
                                     position[ 0 ],
                                     position[ 1 ],
                                     dimensions[ 0 ],
                                     dimensions[ 1 ] ) )
                {
                    if( !( e.stroke.click & CLICK_PRIMARY ) )
                    {
                        setState( OFF_UP );
                        parent -> deassociateDevice( e.stroke.dev_id );
                    }
                }
                else
                {
                    setState( ON_UP );
                    parent -> deassociateDevice( e.stroke.dev_id );
                    return false;                                               // Again, out of button
                }
                break;
            default:
                throw exception( "button::acceptEvent(): Unknown button state" );
            }
            return true;
        default:
            return false;
        }
    }
    
    void button::draw()
    {
        scoped_lock< mutex > slock_e( element_mutex );
        scoped_lock< mutex > slock_r( button_rsrc_mutex );
        
        state_set* top_set;
        state_set* center_set;
        state_set* bottom_set;
        
        switch( state )
        {
        case OFF_UP:
            top_set    = &button_set.rounded_set.off_up;
            center_set = &button_set.rounded_set.off_up;
            bottom_set = &button_set.rounded_set.off_up;
            break;
        case OFF_DOWN:
            top_set    = &button_set.rounded_set.off_down;
            center_set = &button_set.rounded_set.off_down;
            bottom_set = &button_set.rounded_set.off_down;
            break;
        case ON_UP:
            top_set    = &button_set.rounded_set.on_up;
            center_set = &button_set.rounded_set.on_up;
            bottom_set = &button_set.rounded_set.on_up;
            break;
        case ON_DOWN:
            top_set    = &button_set.rounded_set.on_down;
            center_set = &button_set.rounded_set.on_down;
            bottom_set = &button_set.rounded_set.on_down;
            break;
        default:
            throw exception( "button::draw(): Unknown state" );
        }
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            glPushMatrix();                                                     // Draw vertical centers
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, 1.0f, 1.0f );
                top_set -> top_center -> draw();
                glTranslatef( 0.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                bottom_set -> bottom_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix();                                                     // Draw horizontal centers
            {
                glTranslatef( 0.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( 1.0f, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                center_set -> center_left -> draw();
                glTranslatef( dimensions[ 0 ] - BUTTON_MIN_WIDTH / 2.0f, 0.0f, 0.0f );
                center_set -> center_right -> draw();
            }
            glPopMatrix();
            
            glPushMatrix();                                                     // Center
            {
                glTranslatef( BUTTON_MIN_WIDTH / 2.0f, BUTTON_MIN_HEIGHT / 2.0f, 0.0f );
                glScalef( dimensions[ 0 ] - BUTTON_MIN_WIDTH, dimensions[ 1 ] - BUTTON_MIN_HEIGHT, 1.0f );
                center_set -> center_center -> draw();
            }
            glPopMatrix();
            
            glPushMatrix();                                                     // Corners
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
        
        if( contents )
        {
            glPushMatrix();
            {
                switch( contents_align )
                {
                case TOP_LEFT:
                case CENTER_LEFT:
                case BOTTOM_LEFT:
                    glTranslatef( position[ 0 ], 0.0f, 0.0f );
                    break;
                case TOP_CENTER:
                case CENTER_CENTER:
                case BOTTOM_CENTER:
                    glTranslatef( position[ 0 ] + floor( ( dimensions[ 0 ] - contents -> getDimensions().first ) / 2.0f ), 0.0f, 0.0f );
                    break;
                case TOP_RIGHT:
                case CENTER_RIGHT:
                case BOTTOM_RIGHT:
                    glTranslatef( position[ 0 ] + dimensions[ 0 ] - contents -> getDimensions().first, 0.0f, 0.0f );
                    break;
                default:
                    throw exception( "button::draw(): Uknown contents alignment" );
                }
                
                switch( contents_align )
                {
                case TOP_LEFT:
                case TOP_CENTER:
                case TOP_RIGHT:
                    glTranslatef( 0.0f, position[ 1 ], 0.0f );
                    break;
                case CENTER_LEFT:
                case CENTER_CENTER:
                case CENTER_RIGHT:
                    glTranslatef( 0.0f, position[ 1 ] + floor( ( dimensions[ 1 ] - contents -> getDimensions().second ) / 2.0f ), 0.0f );
                    break;
                case BOTTOM_LEFT:
                case BOTTOM_CENTER:
                case BOTTOM_RIGHT:
                    glTranslatef( 0.0f, position[ 1 ] + dimensions[ 1 ] - contents -> getDimensions().second, 0.0f );
                    break;
                default:
                    throw exception( "button::draw(): Uknown contents alignment" );
                }
                
                contents -> draw();
            }
            glPopMatrix();
        }
    }
    
    void button::setState( button_state s )
    {
        switch( s )
        {
        case OFF_UP:
            if( state != OFF_UP
                && state != OFF_DOWN
                && toggle_off_callback )                                        // Change from on to off
            {
                toggle_off_callback -> call();
            }
            break;
        case ON_UP:
            if( state != ON_UP
                && state != ON_DOWN
                && toggle_on_callback )                                         // Change from off to on
            {
                toggle_on_callback -> call();
            }
        default:
            break;
        }
        
        state = s;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
}


