/* 
 * jb_dial.cpp
 * 
 * Implements jade::dial GUI element class
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_dial.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include "jb_named_resources.hpp"
#include "jb_resource.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_settings.hpp"
#include "../windowsys/jb_window.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex dial_rsrc_mutex;
    bool got_resources = false;
    
    struct dial_set
    {
        jade::gui_resource* dial;
        jade::gui_resource* dot;
    };
    
    struct
    {
        dial_set large;
        dial_set small;
    } size_set;
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    dial::dial( dpi::points x,
                dpi::points y,
                bool s,
                float v ) : gui_element( x,
                                         y,
                                         s ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER,
                                         s ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER )
    {
        value = v;
        small = s;
        capturing = NONE;
        
        scoped_lock< mutex > slock( dial_rsrc_mutex );
        
        if( !got_resources )
        {
            size_set.large.dial = getNamedResource( dial_large_dial );
            size_set.large.dot  = getNamedResource( dial_large_dot  );
            size_set.small.dial = getNamedResource( dial_small_dial );
            size_set.small.dot  = getNamedResource( dial_small_dot  );
            
            got_resources = true;
        }
    }
    
    void dial::setValue( float v )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        float old_value = value;
        
        if( v > DIAL_MAX_VALUE )
            value = DIAL_MAX_VALUE;
        else
        {
            if( v < DIAL_MIN_VALUE )
                value = DIAL_MIN_VALUE;
            else
                value = v;
        }
        
        if( value_change_callback )
            value_change_callback -> call();
        
        if( value != old_value && parent != NULL )
            parent -> requestRedraw();
    }
    float dial::getValue()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return value;
    }
    
    void dial::setSmall( bool s )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        small = s;
        
        dimensions[ 0 ] = small ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER;
        dimensions[ 1 ] = small ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    bool dial::getSmall()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return small;
    }
    
    void dial::setValueChangeCallback( const std::shared_ptr< callback >& cb )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        value_change_callback = cb;
    }
    
    bool dial::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dpi::points radius = dimensions[ 0 ] / 2.0f;
        
        if( e.type == STROKE )
        {
            if( capturing != NONE )
            {
                if( e.stroke.dev_id != captured_dev )                           // Ignore but accept other devices wile capturing another
                    return true;
                
                if( !( e.stroke.click & CLICK_PRIMARY ) )                       // Capture cancelled
                {
                    capturing = NONE;
                    deassociateDevice( e.stroke.dev_id );
                    return false;                                               // Don't accept event because it wasn't meant for the dial
                }
                else
                {
                    if( capturing == VERTICAL )
                        setValue( capture_start[ 2 ]
                                  + ( e.position[ 1 ] - capture_start[ 1 ] )
                                  / ( getSetting_num( "jb_DialDragFactor" ) * -1.0f ) );
                    else
                        // atan2 takes y/x, but since our origin is at the top,
                        // we switch that around and multiply y by -1
                        setValue( capture_start[ 2 ]
                                  + atan2(   e.position[ 0 ] - position[ 0 ] - radius,
                                           ( e.position[ 1 ] - position[ 1 ] - radius ) * -1.0f ) / M_PI );
                    
                    if( parent != NULL )
                        parent -> requestRedraw();
                    
                    return true;
                }
            }
            else
            {
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideCircle( e.position[ 0 ],
                                          e.position[ 1 ],
                                          radius,
                                          radius,
                                          radius ) )
                {
                    bool dial_circular_manip;
                    if( !getSetting( "jb_DialCircularManip", dial_circular_manip ) )
                        dial_circular_manip = true;
                    
                    if( !dial_circular_manip
                        || small
                        || pointInsideCircle( e.position[ 0 ],
                                              e.position[ 1 ],
                                              radius,
                                              radius,
                                              radius - 15.0f ) )
                    {
                        capturing = VERTICAL;
                    }
                    else
                    {
                        capturing = CIRCULAR;
                    }
                    
                    capture_start[ 0 ] = e.position[ 0 ];
                    capture_start[ 1 ] = e.position[ 1 ];
                    capture_start[ 2 ] = value;
                    
                    associateDevice( e.stroke.dev_id );
                    captured_dev = e.stroke.dev_id;
                    
                    // Don't need a redraw request
                    
                    return true;
                }
            }
        }
        else
        {
            if( e.type == SCROLL
                && pointInsideCircle( e.position[ 0 ],
                                      e.position[ 1 ],
                                      radius,
                                      radius,
                                      radius ) )
            {
                setValue( value + e.scroll.amount[ 1 ] / ( getSetting_num( "jb_DialDragFactor" ) * getSetting_num( "jb_ScrollDistance" ) / 2.0f ) );
                if( parent != NULL )
                    parent -> requestRedraw();
                return true;
            }
        }
        
        return false;
    }
    
    void dial::draw( window* w )
    {
        scoped_lock< mutex > slock_e( element_mutex );
        scoped_lock< mutex > slock_r( dial_rsrc_mutex );
        
        // TODO: Reference instead?
        dial_set* set = small ? &size_set.small : &size_set.large;
        
        std::pair< dpi::points, dpi::points > dot_dimensions = set -> dot -> getDimensions();
        dpi::points radius = dimensions[ 0 ] / 2.0f;
        dpi::points dot_roff = small ? 7.0f : 9.0f;
        dpi::points dot_pos[ 2 ];
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            set -> dial -> draw( w );
            
            dot_pos[ 0 ] = ( dpi::points )cos( ( value - 0.5f ) * M_PI ) * ( radius - dot_roff );
            dot_pos[ 1 ] = ( dpi::points )sin( ( value - 0.5f ) * M_PI ) * ( radius - dot_roff );
            
            glPushMatrix();
            {
                glTranslatef( dot_pos[ 0 ] + radius - dot_dimensions.first / 2.0f,
                              dot_pos[ 1 ] + radius - dot_dimensions.second / 2.0f,
                              0.0f );
                
                set -> dot -> draw( w );
            }
            glPopMatrix();
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
    
    void dial::clearDeviceAssociations()                                        // clearDeviceAssociations() is not required to be thread-safe
    {
        switch( capturing )
        {
        case NONE:
            break;
        case VERTICAL:
        case CIRCULAR:
            deassociateDevice( captured_dev );
            break;
        default:
            throw exception( "dial::clearDeviceAssociations(): Unknown capturing state" );
        }
    }
}


