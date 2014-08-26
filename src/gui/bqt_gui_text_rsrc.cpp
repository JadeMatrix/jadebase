/* 
 * bqt_gui_text_rsrc.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_text_rsrc.hpp"

#include <cmath>

#include <pango/pangocairo.h>

#include "../bqt_launchargs.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_gl.hpp"

/******************************************************************************//******************************************************************************/

// These values don't really matter as long as they are >= 0 and within machine
// limits, so make them as small as possible.
#define PANGOCAIRO_INITIAL_PX_WIDTH  1
#define PANGOCAIRO_INITIAL_PX_HEIGHT 1

namespace bqt
{
    struct text_update_context
    {
        cairo_status_t        c_status;
        cairo_surface_t*      c_surf;
        cairo_t*              c_context;
        cairo_font_options_t* c_fontops;
        
        PangoLayout*          p_layout;
        PangoFontDescription* p_fontd;
    };
    
    void text_rsrc::updatePixels()
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        text_update_context context;
        
        // Set up Cairo then Pango with initial values /////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        if( max_dimensions[ 0 ] < 0 )
            dimensions[ 0 ] = PANGOCAIRO_INITIAL_PX_WIDTH;
        else
            dimensions[ 0 ] = max_dimensions[ 0 ];
        if( max_dimensions[ 1 ] < 0 )
            dimensions[ 1 ] = PANGOCAIRO_INITIAL_PX_HEIGHT;
        else
            dimensions[ 1 ] = max_dimensions[ 1 ];
        
        updatePixels_setup( &context );
        
        // Redo everything if we need to resize ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int y_off;                                                              // For offsetting the baseline
        
        if( max_dimensions[ 0 ] < 0
            || max_dimensions[ 1 ] < 0 )
        {
            // Get Pango pixel dimensions //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            PangoRectangle p_layout_inkrect;
            
            pango_layout_get_extents( context.p_layout,
                                      &p_layout_inkrect,
                                      NULL );
            
            // Make sure we have enough room
            dimensions[ 0 ] = ceil( ( double )( p_layout_inkrect.width + p_layout_inkrect.x ) / PANGO_SCALE );
            dimensions[ 1 ] = ceil( ( double )( p_layout_inkrect.height + p_layout_inkrect.y ) / PANGO_SCALE );
            
            y_off = p_layout_inkrect.y / PANGO_SCALE;
            
            // Clean up Pango then Cairo ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            updatePixels_cleanup( &context );
            
            // Re-set up Cairo then Pango with actual values ///////////////////////////////////////////////////////////////////////////////////////////////////
            
            updatePixels_setup( &context );
        }
        
        // Draw text to surface ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        pango_cairo_show_layout( context.c_context,
                                 context.p_layout );                            // Render text
        
        tex_offset[ 0 ] = 0;
        tex_offset[ 1 ] = ( pango_layout_get_baseline( context.p_layout ) / PANGO_SCALE * -1 ) + y_off;
        
        // Convert Cairo surface to RGBA for OpenGL ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        if( pixel_space != NULL )
            delete[] pixel_space;
        
        pixel_space = new unsigned char[ dimensions[ 0 ] * dimensions [ 1 ] * 4 ];
        
        if( pixel_space == NULL )
            throw exception( "text_rsrc::updatePixels(): Could not allocate pixel space" );
        
        unsigned char* c_data = cairo_image_surface_get_data( context.c_surf );
        
        int c_stride = cairo_image_surface_get_stride( context.c_surf );
        unsigned char* c_pixelp;
        
        for( long i = 0; i < dimensions[ 0 ] * dimensions[ 1 ]; ++i )
        {
            if( i % dimensions[ 0 ] == 0 )
                c_pixelp = c_data + c_stride * ( i / dimensions[ 0 ] );         // Important since the surface stride might be wider than the surface width
            
            pixel_space[ i * 4 + 0 ] = 0xFF;
            pixel_space[ i * 4 + 1 ] = 0xFF;
            pixel_space[ i * 4 + 2 ] = 0xFF;
            pixel_space[ i * 4 + 3 ] = c_pixelp[ i % dimensions[ 0 ] ];
        }
        
        // Clean up Pango then Cairo ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        updatePixels_cleanup( &context );
    }
    void text_rsrc::updateTexture()
    {
        if( pixel_space == NULL )
            throw exception( "text_rsrc::updateTexture(): Pixel space NULL" );
        
        gl_tex = bytesToTexture( pixel_space,
                                 dimensions[ 0 ],
                                 dimensions[ 1 ],
                                 gl_tex );
        
        delete[] pixel_space;
        pixel_space = NULL;
        update_tex = false;
    }
    
    void text_rsrc::updatePixels_setup( text_update_context* context )
    {
        // Set up Cairo then Pango with initial values /////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        context -> c_surf = cairo_image_surface_create( CAIRO_FORMAT_A8,        // We only need alpha, coloring is handled by OpenGL
                                                        dimensions[ 0 ],
                                                        dimensions[ 1 ] );
        context -> c_status = cairo_surface_status( context -> c_surf );
        if( context -> c_status )
        {
            exception e;
            ff::write( *e,
                       "text_rsrc::updatePixels(): Error creating Cairo surface: ",
                       cairo_status_to_string( context -> c_status ) );
            throw e;
        }
        
        context -> c_context = cairo_create( context -> c_surf );
        context -> c_status = cairo_status( context -> c_context );
        if( context -> c_status )
        {
            exception e;
            ff::write( *e,
                       "text_rsrc::updatePixels(): Error creating Cairo context: ",
                       cairo_status_to_string( context -> c_status ) );
            throw e;
        }
        cairo_surface_destroy( context -> c_surf );                                // Dereference surface
         
        context -> p_layout = pango_cairo_create_layout( context -> c_context );
        
        // Customize Pango layout & font ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        if( max_dimensions[ 0 ] < 0 )
            pango_layout_set_width( context -> p_layout,
                                    -1 );
        else
            pango_layout_set_width( context -> p_layout,
                                    max_dimensions[ 0 ] * PANGO_SCALE );
        if( max_dimensions[ 1 ] < 0 )
            pango_layout_set_height( context -> p_layout,
                                     -1 );
        else
            pango_layout_set_height( context -> p_layout,
                                     max_dimensions[ 1 ] * PANGO_SCALE );
        
        context -> c_fontops = cairo_font_options_create();
        
        if( hinting_enabled )
            cairo_font_options_set_hint_style( context -> c_fontops,
                                               CAIRO_HINT_STYLE_DEFAULT );
        else
            cairo_font_options_set_hint_style( context -> c_fontops,
                                               CAIRO_HINT_STYLE_NONE );
        
        if( antialiasing_enabled )
            cairo_font_options_set_antialias( context -> c_fontops,
                                              CAIRO_ANTIALIAS_DEFAULT );
        else
            cairo_font_options_set_antialias( context -> c_fontops,
                                              CAIRO_ANTIALIAS_NONE );
        
        // TODO: Potentially set subpixel rendering
        
        pango_cairo_context_set_font_options( pango_layout_get_context( context -> p_layout ),
                                              context -> c_fontops );           // Many thanks to ui/gfc/pango_util.cc from the Chromium project, which appears
                                                                                // to be the only online example of setting PangoCairo font options
        
        context -> p_fontd = pango_font_description_from_string( font.c_str() );
        
        pango_font_description_set_absolute_size( context -> p_fontd,
                                                  point_size * PANGO_SCALE );
        
        pango_layout_set_font_description( context -> p_layout,
                                           context -> p_fontd );
        pango_font_description_free( context -> p_fontd );                      // Dereference font description
        
        switch( ellipsize )
        {
            case NONE:
                pango_layout_set_ellipsize( context -> p_layout,
                                            PANGO_ELLIPSIZE_NONE );
                break;
            case BEGINNING:
                pango_layout_set_ellipsize( context -> p_layout,
                                            PANGO_ELLIPSIZE_START );
                break;
            case MIDDLE:
                pango_layout_set_ellipsize( context -> p_layout,
                                            PANGO_ELLIPSIZE_MIDDLE );
                break;
            case END:
                pango_layout_set_ellipsize( context -> p_layout,
                                            PANGO_ELLIPSIZE_END );
                break;
            default:
                throw exception( "text_rsrc::updatePixels(): Unknown ellipsize mode" );
        }
        
        pango_layout_set_text( context -> p_layout,
                               string.c_str(),
                               -1 );
        
        pango_cairo_update_layout( context -> c_context,
                                   context -> p_layout );
    }
    void text_rsrc::updatePixels_cleanup( text_update_context* context )
    {
        // Clean up Pango then Cairo ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        cairo_destroy( context -> c_context );
        
        g_object_unref( context -> p_layout );
        
        cairo_font_options_destroy( context -> c_fontops );
    }
    
    text_rsrc::text_rsrc( window& parent,
                          float p,
                          std::string f,
                          std::string s ) : gui_resource( parent, 0, 0 )
    {
        tex_offset[ 0 ] = 0;
        tex_offset[ 1 ] = 0;
        
        string = s;
        font = f;
        point_size = p;
        
        color[ 0 ] = 1.0f;
        color[ 1 ] = 1.0f;
        color[ 2 ] = 1.0f;
        color[ 3 ] = 1.0f;
        
        max_dimensions[ 0 ] = -1;
        max_dimensions[ 1 ] = -1;
        // max_dimensions[ 0 ] = 24;
        // max_dimensions[ 1 ] = 24;
        
        ellipsize = NONE;
        hinting_enabled = false;
        antialiasing_enabled = true;
        
        pixel_space = NULL;
        gl_tex = 0x00;
        
        update_tex = true;
        updatePixels();
    }
    text_rsrc::~text_rsrc()
    {
        if( pixel_space != NULL )
            delete pixel_space;
    }
    
    float text_rsrc::getPointSize()
    {
        scoped_lock< rwlock > slock( text_lock, RW_READ );
        
        return point_size;
    }
    void text_rsrc::setPointSize( float p )
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        if( p <= 0 )
            throw exception( "text_rsrc::setPointSize(): Point size <= 0" );
        
        point_size = p;
        
        // TODO: While setString() will almost always be called from the main
        // thread (element creation, changing due to events), there may be cases
        // where it is not.  Fix this if it becomes a problem.
        
        update_tex = true;
        updatePixels();
    }
    
    std::string text_rsrc::getString()
    {
        scoped_lock< rwlock > slock( text_lock, RW_READ );
        
        return string;
    }
    void text_rsrc::setString( std::string s )
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        string = s;
        
        // TODO: While setString() will almost always be called from the main
        // thread (element creation, changing due to events), there may be cases
        // where it is not.  Fix this if it becomes a problem.
        
        update_tex = true;
        updatePixels();
    }
    
    // const float* text_rsrc::getColor();
    void text_rsrc::setColor( float r, float g, float b, float a )
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        color[ 0 ] = r;
        color[ 1 ] = g;
        color[ 2 ] = b;
        color[ 3 ] = a;
        
        // Do not update pixels, as coloring is render-time only
    }
    
    std::pair< int, int > text_rsrc::getMaxDimensions()
    {
        scoped_lock< rwlock > slock( text_lock, RW_READ );
        
        return std::pair< int, int >( max_dimensions[ 0 ], max_dimensions[ 1 ] );
    }
    void text_rsrc::setMaxDimensions( int w, int h, ellipsis_mode e )
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        if( w < 0 )
        {
            if( w < -1 )
                throw exception( "text_rsrc::setMaxWidth(): Width < -1" );
            
            max_dimensions[ 0 ] = -1;
            ellipsize = NONE;
        }
        else
        {
            max_dimensions[ 0 ] = w;
            ellipsize = e;
        }
        
        max_dimensions[ 1 ] = h;
        
        update_tex = true;
        updatePixels();
    }
    
    bool text_rsrc::getHinting()
    {
        scoped_lock< rwlock > slock( text_lock, RW_READ );
        
        return hinting_enabled;
    }
    void text_rsrc::setHinting( bool h )
    {
        scoped_lock< rwlock > slock( text_lock, RW_WRITE );
        
        hinting_enabled = h;
        
        update_tex = true;
        updatePixels();
    }
    
    void text_rsrc::draw()
    {
        scoped_lock< rwlock > slock( text_lock, RW_READ );
        
        if( update_tex )
            updateTexture();
        
        glColor4f( color[ 0 ], color[ 1 ], color[ 2 ], color[ 3 ] );
        {
            glBindTexture( GL_TEXTURE_2D, gl_tex );
            
            glTranslatef( tex_offset[ 0 ], tex_offset[ 1 ], 0.0f );
            
            glBegin( GL_QUADS );
            {
                glTexCoord2f( 0.0f, 0.0f );
                glVertex2f( 0.0f, 0.0f );
                
                glTexCoord2f( 0.0f, 1.0f );
                glVertex2f( 0.0f, dimensions[ 1 ] );
                
                glTexCoord2f( 1.0f, 1.0f );
                glVertex2f( dimensions[ 0 ], dimensions[ 1 ] );
                
                glTexCoord2f( 1.0f, 0.0f );
                glVertex2f( dimensions[ 0 ], 0.0f );
            }
            glEnd();
            
            glTranslatef( tex_offset[ 0 ] * -1.0f, tex_offset[ 1 ] * -1.0f, 0.0f );
            
            glBindTexture( GL_TEXTURE_2D, 0x00 );
        }
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    }
}


