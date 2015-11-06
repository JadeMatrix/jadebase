/* 
 * jb_text_rsrc.cpp
 * 
 * Implements jade::text_rsrc class
 * 
 */

// TODO: Update pixels on draw rather than update?  (Pango might not like rendering off the main thread)

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_text_rsrc.hpp"

#include <cmath>

#include <pango/pangocairo.h>

#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

// These values don't really matter as long as they are >= 0 and within machine
// limits, so make them as small as possible.
#define PANGOCAIRO_INITIAL_PX_WIDTH  1
#define PANGOCAIRO_INITIAL_PX_HEIGHT 1

namespace jade
{
    struct text_update_context
    {
        cairo_status_t        c_status;
        cairo_surface_t*      c_surf;
        cairo_t*              c_context;
        cairo_font_options_t* c_fontops;
        
        PangoLayout*          p_layout;
        PangoFontDescription* p_fontd;
        
        unsigned char* pixel_space[ 2 ];
        unsigned int levels;                                                    // 1 = 1x only, 2 = 2x only, 3 = both
        
        text_update_context()
        {
            c_surf = NULL;
            c_context = NULL;
            c_fontops = NULL;
            
            p_layout = NULL;
            p_fontd = NULL;
            
            pixel_space[ 0 ] = NULL;
            pixel_space[ 1 ] = NULL;
            levels = 0;
        }
    };
    
    text_rsrc::text_rsrc( float p,
                          std::string f,
                          std::string s ) : gui_resource( 0, 0 )
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
        
        enable_baseline = true;
        
        ellipsize = NONE;
        hinting_enabled = false;
        antialiasing_enabled = true;
        
        texture = NULL;
        
        updatePixels();
    }
    text_rsrc::~text_rsrc()
    {
        if( texture != NULL )
            releaseTexture( texture );
    }
    
    float text_rsrc::getPointSize()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return point_size;
    }
    void text_rsrc::setPointSize( float p )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        if( p <= 0 )
            throw exception( "text_rsrc::setPointSize(): Point size <= 0" );
        
        point_size = p;
        
        updatePixels();
    }
    
    std::string text_rsrc::getString()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return string;
    }
    void text_rsrc::setString( std::string s )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        string = s;
        
        updatePixels();
    }
    
    std::string text_rsrc::getFont()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return font;
    }
    void text_rsrc::setFont( std::string f )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        font = f;
        
        updatePixels();
    }
    
    void text_rsrc::setColor( float r, float g, float b, float a )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        color[ 0 ] = r;
        color[ 1 ] = g;
        color[ 2 ] = b;
        color[ 3 ] = a;
        
        // Do not update pixels, as coloring is render-time only
    }
    
    std::pair< dpi::points, dpi::points > text_rsrc::getMaxDimensions()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return std::pair< dpi::points, dpi::points >( max_dimensions[ 0 ], max_dimensions[ 1 ] );
    }
    void text_rsrc::setMaxDimensions( dpi::points w,
                                      dpi::points h,
                                      ellipsis_mode e )
    {
        // TODO: Clean up & document this
        
        scoped_lock< mutex > slock( text_mutex );
        
        if( w < 0 )
        {
            if( w < -1 )
                throw exception( "text_rsrc::setMaxDimensions(): Width < -1" );
            
            max_dimensions[ 0 ] = -1;
            ellipsize = NONE;
        }
        else
        {
            max_dimensions[ 0 ] = w;
            ellipsize = e;
        }
        
        max_dimensions[ 1 ] = h;
        
        updatePixels();
    }
    
    bool text_rsrc::getEnableBaseline()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return enable_baseline;
    }
    void text_rsrc::setEnableBaseline( bool b )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        enable_baseline = b;
    }
    
    bool text_rsrc::getHinting()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return hinting_enabled;
    }
    void text_rsrc::setHinting( bool h )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        hinting_enabled = h;
        
        updatePixels();
    }
    
    bool text_rsrc::getAntialiasing()
    {
        scoped_lock< mutex > slock( text_mutex );
        
        return antialiasing_enabled;
    }
    void text_rsrc::setAntialiasing( bool a )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        antialiasing_enabled = a;
        
        updatePixels();
    }
    
    void text_rsrc::draw( window* w )
    {
        scoped_lock< mutex > slock( text_mutex );
        
        if( texture != NULL )
        {
            glColor4f( color[ 0 ], color[ 1 ], color[ 2 ], color[ 3 ] );
            {
                glBindTexture( GL_TEXTURE_2D, texture -> gl_texture );
                
                if( enable_baseline )
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
                
                if( enable_baseline )
                    glTranslatef( tex_offset[ 0 ] * -1.0f,
                                  tex_offset[ 1 ] * -1.0f,
                                  0.0f );
                
                glBindTexture( GL_TEXTURE_2D, 0x00 );
            }
            glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        }
    }
    
    void text_rsrc::updatePixels()
    {
        // WARNING: While setString() will almost always be called from the main
        // thread (element creation, changing due to events), there may be cases
        // where it is not.  Fix this if it becomes a problem.
        
        scoped_lock< mutex > slock( text_mutex );
        
        // Set up Cairo then Pango with initial values /////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        text_update_context context;
        
        if( texture != NULL )
            releaseTexture( texture );
        
        dimensions[ 0 ] = 0;
        dimensions[ 1 ] = 0;
        
        updatePixels_setup( context );
        
        // Get real dimensions /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        PangoRectangle p_layout_lrect;
        
        pango_layout_get_extents( context.p_layout,
                                  NULL,
                                  &p_layout_lrect );                            // Get the logical rect, since the ink rect truncates some stuff
        
        // Make sure we have enough room
        dimensions[ 0 ] = ceil( ( p_layout_lrect.width  + p_layout_lrect.x ) / ( double )( PANGO_SCALE * 2.0f ) );
        dimensions[ 1 ] = ceil( ( p_layout_lrect.height + p_layout_lrect.y ) / ( double )( PANGO_SCALE * 2.0f ) );
        
        if( max_dimensions[ 0 ] > 0
            && dimensions[ 0 ] > max_dimensions[ 0 ] )
            dimensions[ 0 ] = max_dimensions[ 0 ];
        if( max_dimensions[ 1 ] > 0
            && dimensions[ 1 ] > max_dimensions[ 1 ] )
            dimensions[ 1 ] = max_dimensions[ 1 ];
        
        // Redo everything if we need to resize ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        if( max_dimensions[ 0 ] < 0
            || max_dimensions[ 1 ] < 0 )
        {
            // Clean up Pango then Cairo ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            updatePixels_cleanup( context );
            
            // Re-set up Cairo then Pango with actual values ///////////////////////////////////////////////////////////////////////////////////////////////////
            
            updatePixels_setup( context );
        }
        
        // Draw text to surface ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        pango_cairo_show_layout( context.c_context,
                                 context.p_layout );                            // Render text
        
        // TODO: Support vertical baseline
        
        // switch( pango_context_get_gravity( pango_layout_get_context( context.p_layout ) ) )
        // {
        // case PANGO_GRAVITY_EAST:
        //     tex_offset[ 0 ] = pango_layout_get_baseline( context.p_layout ) / ( PANGO_SCALE * -1 * 2.0f );
        //     tex_offset[ 1 ] = 0;
        //     break;
        // case PANGO_GRAVITY_WEST:
        //     tex_offset[ 0 ] = pango_layout_get_baseline( context.p_layout ) / ( PANGO_SCALE * -1 * 2.0f );
        //     tex_offset[ 1 ] = 0;
        //     break;
        // case PANGO_GRAVITY_NORTH:
        //     tex_offset[ 0 ] = 0;
        //     tex_offset[ 1 ] = pango_layout_get_baseline( context.p_layout ) / ( PANGO_SCALE * -1 * 2.0f );
        //     break;
        // case PANGO_GRAVITY_SOUTH:
            tex_offset[ 0 ] = 0;
            tex_offset[ 1 ] = pango_layout_get_baseline( context.p_layout ) / ( PANGO_SCALE * -1 * 2.0f );
        //     break;
        // case PANGO_GRAVITY_AUTO:
        // default:
        //     throw exception( "text_rsrc::updatePixels(): Pango gravity is non-cardinal direction" );
        // }
        
        // Convert Cairo surface to RGBA for OpenGL ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        dpi::pixels pixel_dims[ 2 ];
        pixel_dims[ 0 ] = ceil( dimensions[ 0 ] * 2.0f );                       // Using ceil() as we want the texture at least as big as the points
        pixel_dims[ 1 ] = ceil( dimensions[ 1 ] * 2.0f );
        
        context.pixel_space[ 1 ] = new unsigned char[ pixel_dims[ 0 ] * pixel_dims[ 1 ] * 4 ];
        
        if( context.pixel_space[ 1 ] == NULL )
            throw exception( "text_rsrc::updatePixels(): Could not allocate pixel space" );
        
        unsigned char* c_data = cairo_image_surface_get_data( context.c_surf );
        
        int c_stride = cairo_image_surface_get_stride( context.c_surf );
        unsigned char* c_pixelp;
        
        for( long i = 0; i < pixel_dims[ 0 ] * pixel_dims[ 1 ]; ++i )
        {
            if( i % pixel_dims[ 0 ] == 0 )
                c_pixelp = c_data + c_stride * ( i / pixel_dims[ 0 ] );         // Important since the surface stride might be wider than the surface width
            
            context.pixel_space[ 1 ][ i * 4 + 0 ] = 0xFF;
            context.pixel_space[ 1 ][ i * 4 + 1 ] = 0xFF;
            context.pixel_space[ 1 ][ i * 4 + 2 ] = 0xFF;
            context.pixel_space[ 1 ][ i * 4 + 3 ] = c_pixelp[ i % pixel_dims[ 0 ] ];
        }
        
        texture = acquireTexture( dimensions[ 0 ],
                                  dimensions[ 1 ],
                                  context.pixel_space );
        
        // Clean up Pango then Cairo ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        updatePixels_cleanup( context );
    }
    
    void text_rsrc::updatePixels_setup( text_update_context& context )
    {
        dpi::pixels pixel_dims[ 2 ];
        pixel_dims[ 0 ] = ceil( dimensions[ 0 ] * 2.0f );
        pixel_dims[ 1 ] = ceil( dimensions[ 1 ] * 2.0f );
        
        // Set up Cairo then Pango with initial values /////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        context.c_surf = cairo_image_surface_create( CAIRO_FORMAT_A8,           // We only need alpha, coloring is handled by OpenGL
                                                     pixel_dims[ 0 ],
                                                     pixel_dims[ 1 ] );
        context.c_status = cairo_surface_status( context.c_surf );
        if( context.c_status )
        {
            exception e;
            ff::write( *e,
                       "text_rsrc::updatePixels(): Error creating Cairo surface: ",
                       cairo_status_to_string( context.c_status ) );
            throw e;
        }
        
        context.c_context = cairo_create( context.c_surf );
        context.c_status = cairo_status( context.c_context );
        if( context.c_status )
        {
            exception e;
            ff::write( *e,
                       "text_rsrc::updatePixels(): Error creating Cairo context: ",
                       cairo_status_to_string( context.c_status ) );
            throw e;
        }
        cairo_surface_destroy( context.c_surf );                                // Dereference surface
         
        context.p_layout = pango_cairo_create_layout( context.c_context );
        
        // Customize Pango layout & font ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // Working with the logical bounding box here; this allows us to left-
        // justify right-to-left text
        if( pixel_dims[ 0 ] == 0 )
            pango_layout_set_width( context.p_layout,
                                    -1 );
        else
            pango_layout_set_width( context.p_layout,
                                    pixel_dims[ 0 ] * PANGO_SCALE );
        if( pixel_dims[ 1 ] == 0 )
            pango_layout_set_height( context.p_layout,
                                     -1 );
        else
            pango_layout_set_height( context.p_layout,
                                     pixel_dims[ 1 ] * PANGO_SCALE );
        
        context.c_fontops = cairo_font_options_create();
        
        if( hinting_enabled )
            cairo_font_options_set_hint_style( context.c_fontops,
                                               CAIRO_HINT_STYLE_DEFAULT );
        else
            cairo_font_options_set_hint_style( context.c_fontops,
                                               CAIRO_HINT_STYLE_NONE );
        
        if( antialiasing_enabled )
            cairo_font_options_set_antialias( context.c_fontops,
                                              CAIRO_ANTIALIAS_DEFAULT );
        else
            cairo_font_options_set_antialias( context.c_fontops,
                                              CAIRO_ANTIALIAS_NONE );
        
        // TODO: Potentially set subpixel rendering (requires Pango to handle text color)
        
        pango_cairo_context_set_font_options( pango_layout_get_context( context.p_layout ),
                                              context.c_fontops );              // Many thanks to ui/gfc/pango_util.cc from the Chromium project, which appears
                                                                                // to be the only online example of setting PangoCairo font options
        
        context.p_fontd = pango_font_description_from_string( font.c_str() );
        
        pango_font_description_set_absolute_size( context.p_fontd,
                                                  point_size * 2.0f * PANGO_SCALE );
        
        pango_layout_set_font_description( context.p_layout,
                                           context.p_fontd );
        pango_font_description_free( context.p_fontd );                         // Dereference font description
        
        switch( ellipsize )
        {
            case NONE:
                pango_layout_set_ellipsize( context.p_layout,
                                            PANGO_ELLIPSIZE_NONE );
                break;
            case BEGINNING:
                pango_layout_set_ellipsize( context.p_layout,
                                            PANGO_ELLIPSIZE_START );
                break;
            case MIDDLE:
                pango_layout_set_ellipsize( context.p_layout,
                                            PANGO_ELLIPSIZE_MIDDLE );
                break;
            case END:
                pango_layout_set_ellipsize( context.p_layout,
                                            PANGO_ELLIPSIZE_END );
                break;
            default:
                throw exception( "text_rsrc::updatePixels(): Unknown ellipsize mode" );
        }
        
        pango_layout_set_text( context.p_layout,
                               string.c_str(),
                               -1 );
        
        pango_cairo_update_layout( context.c_context,
                                   context.p_layout );
    }
    void text_rsrc::updatePixels_cleanup( text_update_context& context )
    {
        g_object_unref( context.p_layout );
        
        cairo_destroy( context.c_context );
        
        cairo_font_options_destroy( context.c_fontops );
    }
}


