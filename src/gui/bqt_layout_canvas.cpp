/* 
 * bqt_layout_canvas.cpp
 * 
 * Implements bqt_layout_canvas.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_layout_canvas.hpp"
 
#include "bqt_gl.hpp"
#include "bqt_exception.hpp"

/******************************************************************************//******************************************************************************/

// void drawCircle( unsigned int sides, float radius = 1.0, bool fill = true )
// {
//     ff::write( bqt_out, "Drawing a circle\n" );
    
//     if( fill )
//         glBegin( GL_POLYGON );
//     else
//         glBegin( GL_LINE_LOOP );
//     {
//         glPushMatrix();
//         {
//             // glTranslatef( radius, 0.0, 0.0 );
            
//             for( int i = 0; i < sides; i++ )
//             {
//                 glRotatef( 360.0f / sides, 1.0, 0.0, 0.0 );
                
//                 glVertex2f( radius, 0.0 );
//             }
//         }
//         glPopMatrix();
//     }
//     glEnd();
// }

/******************************************************************************//******************************************************************************/

namespace bqt
{
    /* PANE *******************************************************************//******************************************************************************/
    
    void canvas_element::pane::draw( unsigned int w,
                                     unsigned int h,
                                     bool vert )
    {
        scoped_lock slock( pane_mutex );
        
        switch( type )
        {
            case EMPTY:
                // do nothing
                break;
            case CANVAS:
                // OpenGL stencil
                break;
            case PANES:
                // {
                //     unsigned int offset = 0;
                    
                //     for( int i = 0; i < children.size(); ++i )
                //     {
                //         glTranslatef( ( vert ? offset += w * children[ i ].second : 0 ),
                //                       ( vert ? 0 : offset += h * children[ i ].second ),
                //                       0.0f );
                        
                //         children[ i ].first.draw( w * ( vert ? children[ i ].second : 1.0f ) - __???__,
                //                                   h * ( vert ? 1.0f : children[ i ].second ) - __???__,
                //                                   !vert );
                //     }
                // }
                break;
            default:
                throw exception( "canvas_element::pane::draw(): Invalid pane type" );
                break;
        }
    }
    
    /* CANVAS_ELEMENT *********************************************************//******************************************************************************/
    
    canvas_element::canvas_element( int x,
                                    int y,
                                    unsigned int w,
                                    unsigned int h ) : layout_element( x, y, w, h )
    {
        root.type = pane::EMPTY;
        root.contents = NULL;
    }
    
    layout_element* canvas_element::acceptEvent( window_event& e )
    {
        scoped_lock slock( element_mutex );
        
        switch( e.type )
        {
        case STROKE:
            break;
        case DROP:
            break;
        case KEYCOMMAND:
            break;
        case COMMAND:
            break;
        case TEXT:
            break;
        default:
            throw exception( "canvas_layout::acceptEvent(): Uknown event type" );
            break;
        }
        
        return NULL;
    }
    
    void canvas_element::draw()
    {
        scoped_lock slock( element_mutex );
        
        root.draw( dimensions[ 0 ], dimensions[ 1 ] );
    }
}


