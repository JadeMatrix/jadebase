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

#include "bqt_gui_resource.hpp"
#include "../bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    struct dial_set
    {
        bqt::gui_resource* dial;
        bqt::gui_resource* dot;
    };
    
    struct size_set
    {
        dial_set large;
        dial_set small;
        
        int count;
    };
    
    bqt::rwlock dial_rsrc_lock;
    std::map< bqt::window*, size_set > size_sets;
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
        capturing = false;
        
        scoped_lock< rwlock > slock( dial_rsrc_lock, RW_WRITE );
        
        if( !size_sets.count( &parent ) )
        {
            size_set& window_set( size_sets[ &parent ] );
            
            window_set.large.dial = parent.getNamedResource( dial_large_dial );
            window_set.large.dot  = parent.getNamedResource( dial_large_dot  );
            window_set.small.dial = parent.getNamedResource( dial_small_dial );
            window_set.small.dot  = parent.getNamedResource( dial_small_dot  );
        }
        else
            size_sets[ &parent ].count++;
    }
    dial::~dial()
    {
        scoped_lock< rwlock > slock( dial_rsrc_lock, RW_WRITE );
        
        size_sets[ &parent ].count--;
        
        if( size_sets[ &parent ].count < 1 )
            size_sets.erase( &parent );
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
        
        if( e.type == STROKE )
        {
            if( capturing )
            {
                if( !( e.stroke.click & CLICK_PRIMARY ) )                       // Capture cancelled
                {
                    capturing = false;
                    parent.associateDevice( e.stroke.dev_id, NULL );
                    return false;
                }
                else
                {
                    setValue( ( e.stroke.position[ 1 ] - capture_start[ 1 ] ) / DIAL_DRAG_FACTOR );
                    
                    parent.requestRedraw();
                    
                    return true;
                }
            }
            else
            {
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideCircle( e.stroke.position[ 0 ],
                                          e.stroke.position[ 1 ],
                                          position[ 0 ],
                                          position[ 1 ],
                                          dimensions[ 0 ] ) )
                {
                    capturing = true;
                    
                    capture_start[ 0 ] = e.stroke.position[ 0 ];
                    capture_start[ 1 ] = e.stroke.position[ 1 ];
                    
                    parent.associateDevice( e.stroke.dev_id, this );
                    
                    return true;
                }
                else
                    return false;
            }
        }
        
        if( e.type == KEYCOMMAND && e.key.key == KEY_D && e.key.up )
        {
            value += 0.1f;
            parent.requestRedraw();
            return true;
        }
        
        return false;
    }
    
    void dial::draw()
    {
        scoped_lock< rwlock > slock_e( element_lock, RW_READ );
        scoped_lock< rwlock > slock_r( dial_rsrc_lock, RW_READ );
        
        dial_set* set = small ? &size_sets[ &parent ].small : &size_sets[ &parent ].large;
        
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


