/* 
 * bqt_gui_dial.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_dial.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include "bqt_gui_resource.hpp"
#include "../bqt_preferences.hpp"
#include "bqt_named_resources.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::rwlock dial_rsrc_lock;
    bool got_resources = false;
    
    struct dial_set
    {
        bqt::gui_resource* dial;
        bqt::gui_resource* dot;
    };
    
    struct
    {
        dial_set large;
        dial_set small;
    } size_set;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    dial::dial( window& parent,
                int x,
                int y,
                bool s,
                float v ) : gui_element( parent,
                                         x,
                                         y,
                                         s ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER,
                                         s ? DIAL_SMALL_DIAMETER : DIAL_LARGE_DIAMETER )
    {
        value = v;
        small = s;
        capturing = NONE;
        
        scoped_lock< rwlock > slock( dial_rsrc_lock, RW_WRITE );
        
        if( !got_resources )
        {
            size_set.large.dial = getNamedResource( dial_large_dial );
            size_set.large.dot  = getNamedResource( dial_large_dot  );
            size_set.small.dial = getNamedResource( dial_small_dial );
            size_set.small.dot  = getNamedResource( dial_small_dot  );
            
            got_resources = true;
        }
    }
    
    float dial::getValue()
    {
        scoped_lock< rwlock > slock( element_lock, RW_READ );
        
        return value;
    }
    void dial::setValue( float v )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        if( v > DIAL_MAX_VALUE )
            value = DIAL_MAX_VALUE;
        else
        {
            if( v < DIAL_MIN_VALUE )
                value = DIAL_MIN_VALUE;
            else
                value = v;
        }
        
        parent.requestRedraw();
    }
    
    bool dial::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        float radius = dimensions[ 0 ] / 2.0f;
        
        if( e.type == STROKE )
        {
            if( capturing != NONE )
            {
                if( !( e.stroke.click & CLICK_PRIMARY ) )                       // Capture cancelled
                {
                    capturing = NONE;
                    parent.deassociateDevice( e.stroke.dev_id );
                    return true;                                                // Accept event because we used it
                }
                else
                {
                    if( capturing == VERTICAL )
                        setValue( capture_start[ 2 ]
                                  + ( e.stroke.position[ 1 ] - e.offset[ 1 ] - capture_start[ 1 ] )
                                  / ( DIAL_DRAG_FACTOR * -1.0f ) );
                    else
                        // atan2 takes y/x, but since our origin is at the top,
                        // we switch that around and multiply y by -1
                        setValue( capture_start[ 2 ]
                                  + atan2(   e.stroke.position[ 0 ] - e.offset[ 0 ] - position[ 0 ] - radius,
                                           ( e.stroke.position[ 1 ] - e.offset[ 1 ] - position[ 1 ] - radius ) * -1.0f ) / M_PI );
                    
                    parent.requestRedraw();
                    
                    return true;
                }
            }
            else
            {
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideCircle( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                          e.stroke.position[ 1 ] - e.offset[ 1 ],
                                          position[ 0 ] + radius,
                                          position[ 1 ] + radius,
                                          radius ) )
                {
                    if( !getDialCircularManip()
                        || small
                        || pointInsideCircle( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                              e.stroke.position[ 1 ] - e.offset[ 1 ],
                                              position[ 0 ] + radius,
                                              position[ 1 ] + radius,
                                              radius - 15.0f ) )
                    {
                        capturing = VERTICAL;
                    }
                    else
                    {
                        capturing = CIRCULAR;
                    }
                    
                    capture_start[ 0 ] = e.stroke.position[ 0 ] - e.offset[ 0 ];
                    capture_start[ 1 ] = e.stroke.position[ 1 ] - e.offset[ 1 ];
                    capture_start[ 2 ] = value;
                    
                    parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                    
                    // Don't need a redraw request
                    
                    return true;
                }
            }
        }
        else
        {
            if( e.type == SCROLL
                && pointInsideCircle( e.scroll.position[ 0 ] - e.offset[ 0 ],
                                      e.scroll.position[ 1 ] - e.offset[ 1 ],
                                      position[ 0 ] + radius,
                                      position[ 1 ] + radius,
                                      radius ) )
            {
                setValue( value + e.scroll.amount[ 1 ] / ( DIAL_DRAG_FACTOR * getWheelScrollDistance() / 2.0f ) );
                parent.requestRedraw();
                return true;
            }
        }
        
        return false;
    }
    
    void dial::draw()
    {
        scoped_lock< rwlock > slock_e( element_lock, RW_READ );
        scoped_lock< rwlock > slock_r( dial_rsrc_lock, RW_READ );
        
        dial_set* set = small ? &size_set.small : &size_set.large;
        
        std::pair< unsigned int, unsigned int > dot_dimensions = set -> dot -> getDimensions();
        float radius = dimensions[ 0 ] / 2.0f;
        float dot_roff = small ? 7.0f : 9.0f;
        float dot_pos[ 2 ];
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            set -> dial -> draw();
            
            dot_pos[ 0 ] = ( float )cos( ( value - 0.5f ) * M_PI ) * ( radius - dot_roff );
            dot_pos[ 1 ] = ( float )sin( ( value - 0.5f ) * M_PI ) * ( radius - dot_roff );
            
            glPushMatrix();
            {
                glTranslatef( dot_pos[ 0 ] + radius - dot_dimensions.first / 2.0f,
                              dot_pos[ 1 ] + radius - dot_dimensions.second / 2.0f,
                              0.0f );
                
                set -> dot -> draw();
            }
            glPopMatrix();
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


