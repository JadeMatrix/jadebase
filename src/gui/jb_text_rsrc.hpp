#ifndef JADEBASE_TEXT_RSRC_HPP
#define JADEBASE_TEXT_RSRC_HPP

/* 
 * jb_text_rsrc.hpp
 * 
 * GUI resource type to handle rendering of UTF8 strings with basic formatting
 * 
 * The default text color is white, as it can easily be colored with glColor*();
 * it can be changed with text_rsrc::setColor().
 * 
 * Note that text resources' origins are not at the top left (as are images'),
 * but at the baseline of the first line of text.  This means that for a single
 * line of standard horizontal left-to-right style text (like English) the
 * origin is roughly the lower left corner (if baseline is enabled).  This
 * baseline behavior can be enabled/disabled with setEnableBaseline().
 * 
 * TODO: Consider using dpi::points for the point size
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_resource.hpp"

#include <string>

#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

// These are more suggestions/fallbacks and don't include sizes or colors
#define TEXTBOX_FONT   "LiberationSerif Regular"
#define GUI_LABEL_FONT "LiberationSans Bold"
#define GUI_DEBUG_FONT "LiberationMono Regular"

namespace jade
{
    struct text_update_context;                                                 // Opaque utility struct
    
    class text_rsrc : public gui_resource
    {
    public:
        enum ellipsis_mode
        {
            NONE,
            BEGINNING,
            MIDDLE,
            END
        };
        
        text_rsrc( float p = 12.0f,
                   std::string f = TEXTBOX_FONT,
                   std::string s = "" );
        ~text_rsrc();
        
        float getPointSize();
        void  setPointSize( float );
        
        std::string getString();
        void        setString( std::string );
        
        std::string getFont();
        void        setFont( std::string );                                     // Font name as a string, for example "Menlo Bold"
        
        void setColor( float, float, float, float );                            // RGBA
        
        #define TEXT_MAXWIDTH_NONE      -1
                                                                                // FIXME: Figure out why Pango doesn't like one line:
        // #define TEXT_MAXHEIGHT_ONELINE   0                                      // Doesn't work like it should, perhaps?  Pango docs not clear.
        
        std::pair< dpi::points, dpi::points > getMaxDimensions();
        void setMaxDimensions( dpi::points,                                     // Max width in points
                               dpi::points,                                     // Max heigth in points
                               ellipsis_mode e = NONE );                        // Ellipsis mode for truncating string when it does not fit max dimensions
        
        bool getEnableBaseline();
        void setEnableBaseline( bool );                                         // Centered baseline obeys writing direction (horizontal/vertical)
        
        bool getHinting();
        void setHinting( bool );
        
        bool getAntialiasing();
        void setAntialiasing( bool );
        
        void draw( window* );
        
    protected:
        mutex text_mutex;
        
        dpi::points tex_offset[ 2 ];                                            // Texture offset from the baseline
        
        std::string string;
        std::string font;
        float point_size;
        float color[ 4 ];
        dpi::points max_dimensions[ 2 ];
        bool enable_baseline;
        ellipsis_mode ellipsize;
        bool hinting_enabled;
        bool antialiasing_enabled;
        
        unsigned char* pixel_space;
        GLuint gl_tex;
        
        bool update_tex;
        void updatePixels();
        void updateTexture();
        
        void updatePixels_setup( text_update_context* );
        void updatePixels_cleanup( text_update_context* );
    };
}

/******************************************************************************//******************************************************************************/

#endif


