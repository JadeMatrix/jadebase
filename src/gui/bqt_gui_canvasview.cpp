/* 
 * bqt_gui_canvasview.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_canvasview.hpp"

#include "../bqt_gl.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    canvas_view::canvas_view( window& parent,
                              int x,
                              int y,
                              unsigned int w,
                              unsigned int h,
                              unsigned int c_w,
                              unsigned int c_h ) : scrollable( parent, x, y, w, h )
    {
        // data_dims[ 0 ] = c_w;
        // data_dims[ 1 ] = c_h;
        // // pixel_data = new unsigned char[ c_w * c_h * 4 ];
        // canvas_tex = 0x00;
        // init = true;
        
        // // for( int i = 0; i < c_w * c_h; ++i )
        // // {
        // //     pixel_data[ i * 4 + 0 ] = 0xFF;
        // //     pixel_data[ i * 4 + 1 ] = 0xFF;
        // //     pixel_data[ i * 4 + 2 ] = 0xFF;
        // //     pixel_data[ i * 4 + 3 ] = 0xFF;
        // // }
        
        // if( data_dims[ 0 ] > dimensions[ 0 ] )
        // {
        //     scroll_limits[ 0 ] = dimensions[ 0 ] - data_dims[ 0 ];
        //     scroll_offset[ 0 ] = scroll_limits[ 0 ] / 2;
        // }
        // else
        // {
        //     scroll_limits[ 0 ] = 0;
        //     scroll_offset[ 0 ] = 0;
        // }
        // scroll_limits[ 1 ] = 0;
        
        // if( data_dims[ 1 ] > dimensions[ 1 ] )
        // {
        //     scroll_limits[ 2 ] = dimensions[ 1 ] - data_dims[ 1 ];
        //     scroll_offset[ 1 ] = scroll_limits[ 2 ] / 2;
        // }
        // else
        // {
        //     scroll_limits[ 2 ] = 0;
        //     scroll_offset[ 1 ] = 0;
        // }
        // scroll_limits[ 3 ] = 0;
    }
    canvas_view::~canvas_view()
    {
        // if( pixel_data != NULL )
        //     delete[] pixel_data;
    }
    
    // GUI_ELEMENT /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    bool canvas_view::acceptEvent( window_event& e )
    {
        // scoped_lock< mutex > slock( element_mutex );
        
        // if( e.type == STROKE )
        // {
        //     if( e.stroke.pressure > 0.0f )
        //     {
        //         if( !captured_devs.count( e.stroke.dev_id ) )
        //         {
        //             parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
        //             captured_devs.insert( e.stroke.dev_id );
        //         }
                
        //         ff::write( bqt_out, wevent2str( e ), "\n" );
                
        //         pending_waypoints.push( e.stroke );
        //         // pending_waypoints.back().position[ 0 ] -= e.offset[ 0 ];
        //         // pending_waypoints.back().position[ 1 ] -= e.offset[ 1 ];
                
        //         parent.requestRedraw();
        //     }
        //     else
        //     {
        //         if( captured_devs.count( e.stroke.dev_id ) )
        //         {
        //             parent.deassociateDevice( e.stroke.dev_id );
        //             captured_devs.erase( e.stroke.dev_id );
        //         }
        //     }
        // }
        
        return false;
    }
    
    void canvas_view::draw()
    {
        // scoped_lock< mutex > slock( element_mutex );
        
        // // if( pixel_data != NULL )
        // if( init )
        // {
        //     // canvas_tex = bytesToTexture( pixel_data,
        //     //                              data_dims[ 0 ],
        //     //                              data_dims[ 1 ],
        //     //                              canvas_tex );
        //     // delete[] pixel_data;
        //     // pixel_data = NULL;
            
        //     glGenTextures( 1, &canvas_tex );
        //     if( canvas_tex == 0x00 )
        //         throw exception( "canvas_view::draw(): Could not create texture for FBO" );
        //     glBindTexture( GL_TEXTURE_2D, canvas_tex );
        //     glTexImage2D( GL_TEXTURE_2D,
        //                   0,
        //                   GL_RGBA,
        //                   data_dims[ 0 ],
        //                   data_dims[ 1 ],
        //                   0,
        //                   GL_RGBA,
        //                   GL_UNSIGNED_BYTE,
        //                   NULL );
        //     glBindTexture( GL_TEXTURE_2D, 0x00 );
        //     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        //     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            
        //     glGenFramebuffers( 1, &canvas_fb );
            
        //     if( canvas_fb == 0x00 )
        //         throw exception( "canvas_view::draw(): Could not generate FBO" );
            
        //     glBindFramebuffer( GL_FRAMEBUFFER, canvas_fb );
            
        //     glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvas_tex, 0 );
        //     // glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, canvas_tex, 0 );
            
        //     GLenum gl_drawbuffs[] = { GL_COLOR_ATTACHMENT0 };
        //     glDrawBuffers( ( sizeof( gl_drawbuffs ) / sizeof( GLenum ) ), gl_drawbuffs );
            
        //     if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        //         throw exception( "canvas_view::draw(): Could not finalize FBO" );
            
        //     glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
            
        //     glBindFramebuffer( GL_FRAMEBUFFER, 0x00 );
            
        //     init = false;
        // }
        
        // if( pending_waypoints.size() )
        // {
        //     glPushMatrix();
        //     {
        //         glBindFramebuffer( GL_FRAMEBUFFER, canvas_fb );
                
        //         glViewport( 0, 0, data_dims[ 0 ], data_dims[ 1 ] );
                
        //         glLoadIdentity();
        //         glOrtho( 0.0, data_dims[ 0 ], data_dims[ 1 ], 0.0, 1.0, -1.0 );
                
        //         {
        //             glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
        //             glBegin( GL_QUADS );
        //             {
        //                 glVertex2f( 0.0f, 0.0f );
        //                 glVertex2f( 0.0f, 10.0f );
        //                 glVertex2f( 10.0f, 10.0f );
        //                 glVertex2f( 10.0f, 0.0f );
        //             }
        //             glEnd();
        //             glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        //         }
                
        //         glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
                
        //         glPointSize( 10 );
        //         glBegin( GL_POINTS );
        //         {
        //             for( int i = pending_waypoints.size(); i > 0; --i )
        //             {
        //                 stroke_waypoint& waypoint( pending_waypoints.front() );
                        
        //                 // ff::write( bqt_out,
        //                 //            "Drawing point @ ",
        //                 //            waypoint.position[ 0 ] + scroll_offset[ 0 ],
        //                 //            ", ",
        //                 //            waypoint.position[ 1 ] + scroll_offset[ 1 ],
        //                 //            " : ",
        //                 //            ( int )( waypoint.pressure * 10 ),
        //                 //            "\n" );
                        
        //                 // glPointSize( waypoint.pressure * 10 );
        //                 glVertex2f( 256,
        //                             256 );
        //                 // glVertex2f( waypoint.position[ 0 ] + scroll_offset[ 0 ],
        //                 //             waypoint.position[ 1 ] + scroll_offset[ 1 ] );
                        
        //                 pending_waypoints.pop();
        //             }
        //         }
        //         glEnd();
                
        //         glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
                
        //         glBindFramebuffer( GL_FRAMEBUFFER, 0x00 );
        //     }
        //     glPopMatrix();
        // }
        
        // glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        // {
        //     clearDrawMasks();
            
        //     // addDrawMask( 0, 0, dimensions[ 0 ], dimensions[ 1 ] );
            
        //     glPushMatrix();
        //     {
        //         // glTranslatef( scroll_offset[ 0 ], scroll_offset[ 1 ], 0.0f );
        //         glTranslatef( 20.0f, 20.0f, 0.0f );
                
        //         glBindTexture( GL_TEXTURE_2D, canvas_tex );
                
        //         glBegin( GL_QUADS );
        //         {
        //             glTexCoord2f( 0.0f, 0.0f );
        //             glVertex2f( 0.0f, 0.0f );
                    
        //             glTexCoord2f( 0.0f, 1.0f );
        //             glVertex2f( 0.0f, data_dims[ 1 ] );
                    
        //             glTexCoord2f( 1.0f, 1.0f );
        //             glVertex2f( data_dims[ 0 ], data_dims[ 1 ] );
                    
        //             glTexCoord2f( 1.0f, 0.0f );
        //             glVertex2f( data_dims[ 0 ], 0.0f );
        //         }
        //         glEnd();
                
        //         glBindTexture( GL_TEXTURE_2D, 0x00 );
        //     }
        //     glPopMatrix();
            
        //     // glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
            
        //     // glLineWidth( 4.0f );
            
        //     // glBegin( GL_LINE_LOOP );
        //     // {
        //     //     glVertex2f( 0.0f, 0.0f );
        //     //     glVertex2f( 0.0f, dimensions[ 1 ] );
        //     //     glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
        //     //     glVertex2f( dimensions[ 0 ], 0.0f );
        //     // }
        //     // glEnd();
            
        //     // glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
            
        //     clearDrawMasks();
        // }
        // glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
    
    // SCROLLABLE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void canvas_view::setRealDimensions( unsigned int w, unsigned int h )
    {
        // scoped_lock< mutex > slock( element_mutex );
        
        // dimensions[ 0 ] = w;
        // dimensions[ 1 ] = h;
        
        // ff::write( bqt_out,
        //            "Set canvas view dimensions to ",
        //            dimensions[ 0 ],
        //            ", ",
        //            dimensions[ 1 ],
        //            "\n" );
        
        // if( data_dims[ 0 ] > dimensions[ 0 ] )
        // {
        //     scroll_limits[ 0 ] = dimensions[ 0 ] - data_dims[ 0 ];
            
        //     if( scroll_offset[ 0 ] < scroll_limits[ 0 ] )
        //         scroll_offset[ 0 ] = scroll_limits[ 0 ];
        // }
        // else
        // {
        //     scroll_limits[ 0 ] = 0;
        //     scroll_offset[ 0 ] = 0;
        // }
        // scroll_limits[ 1 ] = 0;
        
        // if( data_dims[ 1 ] > dimensions[ 1 ] )
        // {
        //     scroll_limits[ 2 ] = dimensions[ 1 ] - data_dims[ 1 ];
            
        //     if( scroll_offset[ 1 ] < scroll_limits[ 2 ] )
        //         scroll_offset[ 1 ] = scroll_limits[ 2 ];
        // }
        // else
        // {
        //     scroll_limits[ 2 ] = 0;
        //     scroll_offset[ 1 ] = 0;
        // }
        // scroll_limits[ 3 ] = 0;
        
        // parent.requestRedraw();
    }
    
    void canvas_view::scrollPixels( int x, int y )
    {
        // scoped_lock< mutex > slock( element_mutex );
        
        // if( scroll_limits[ 0 ] > scroll_limits[ 1 ]
        //     || scroll_limits[ 2 ] > scroll_limits[ 3 ] )
        // {
        //     throw exception( "canvas_view::scrollPixels(): Some min limit above max limit" );
        // }
        
        // scroll_offset[ 0 ] += x;
        
        // if( scroll_offset[ 0 ] < scroll_limits[ 0 ] )
        //     scroll_offset[ 0 ] = scroll_limits[ 0 ];
        // else
        //     if( scroll_offset[ 0 ] > scroll_limits[ 1 ] )
        //         scroll_offset[ 0 ] = scroll_limits[ 1 ];
        
        // scroll_offset[ 1 ] += y;
        
        // if( scroll_offset[ 1 ] < scroll_limits[ 2 ] )
        //     scroll_offset[ 1 ] = scroll_limits[ 2 ];
        // else
        //     if( scroll_offset[ 1 ] > scroll_limits[ 3 ] )
        //         scroll_offset[ 1 ] = scroll_limits[ 3 ];
        
        // parent.requestRedraw();
    }
    void canvas_view::scrollPercent( float x, float y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPixels( x * data_dims[ 0 ], y * data_dims[ 1 ] );
    }
    
    void canvas_view::setScrollPixels( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPixels( x - scroll_offset[ 0 ], y - scroll_offset[ 1 ] );
    }
    void canvas_view::setScrollPercent( float x, float y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        scrollPixels( x * data_dims[ 0 ] - scroll_offset[ 0 ],
                      y * data_dims[ 1 ] - scroll_offset[ 1 ] );
    }
    
    std::pair< int, int > canvas_view::getScrollPixels()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< int, int >( scroll_offset[ 0 ], scroll_offset[ 1 ] );
    }
    std::pair< float, float > canvas_view::getScrollPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< float, float >( ( float )scroll_offset[ 0 ] / ( float )data_dims[ 0 ],
                                          ( float )scroll_offset[ 1 ] / ( float )data_dims[ 1 ] );
    }
    
    bool canvas_view::hasScrollLimit()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return true;
    }
    std::pair< int, int > canvas_view::getScrollLimitPixels()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // limit_pixels l;
        
        // l.first.first = scroll_limits[ 0 ];
        // l.first.second = scroll_limits[ 1 ];
        // l.second.first = scroll_limits[ 2 ];
        // l.second.second = scroll_limits[ 3 ];
        
        // return l;
        
        return std::pair< int, int >( scroll_limits[ 0 ], scroll_limits[ 1 ] );
    }
    std::pair< float, float> canvas_view::getScrollLimitPercent()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        // limit_percent l;
        
        // l.first.first   = ( float )scroll_limits[ 0 ] / ( float )data_dims[ 0 ];
        // l.first.second  = ( float )scroll_limits[ 1 ] / ( float )data_dims[ 0 ];
        // l.second.first  = ( float )scroll_limits[ 2 ] / ( float )data_dims[ 1 ];
        // l.second.second = ( float )scroll_limits[ 3 ] / ( float )data_dims[ 1 ];
        
        // return l;
        
        return std::pair< float, float >( ( float )scroll_limits[ 0 ] / data_dims[ 0 ],
                                          ( float )scroll_limits[ 1 ] / data_dims[ 1 ] );
    }
}


