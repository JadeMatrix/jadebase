#ifndef BQT_GUI_TEXT_RSRC_HPP
#define BQT_GUI_TEXT_RSRC_HPP

/* 
 * bqt_gui_text_rsrc.hpp
 * 
 * GUI resource type to handle rendering of UTF8 strings with basic formatting
 * 
 * The default text color is which, as it can easily be colored with glColor*();
 * it can be changed with text_rsrc::setColor().
 * 
 * Note that text resources' origins are not at the top left (as are images'),
 * but at the baseline of the first line of text.  This means that for a single
 * line of standard horizontal left-to-right style text (like English) the
 * origin is roughly the lower left corner.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_resource.hpp"

#include <string>

#include "../threading/bqt_rwlock.hpp"

/******************************************************************************//******************************************************************************/

// These are more suggestions/fallbacks and don't include sizes or colors
#define TEXTBOX_FONT   "LiberationSerif Regular"
#define GUI_LABEL_FONT "LiberationSans Bold"
#define GUI_DEBUG_FONT "LiberationMono Regular"

namespace bqt
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
    protected:
        rwlock text_lock;
        
        int tex_offset[ 2 ];                                                    // Texture offset from the baseline
        
        std::string string;
        std::string font;
        float point_size;
        float color[ 4 ];
        int max_dimensions[ 2 ];
        ellipsis_mode ellipsize;
        bool hinting_enabled;
        bool antialiasing_enabled;
        
        unsigned char* pixel_space;
        GLuint gl_tex;
        
        bool update_tex;
        void updatePixels();
        void updateTexture();
        
        void updatePixels_setup( text_update_context* context );
        void updatePixels_cleanup( text_update_context* context );
    public:
        text_rsrc( window& parent,
                   float p = 12.0f,
                   std::string f = TEXTBOX_FONT,
                   std::string s = "" );
        ~text_rsrc();
        
        float getPointSize();
        void  setPointSize( float p );
        
        std::string getString();
        void        setString( std::string s );
        
        std::string getFont();
        void        setFont( std::string f );
        
        // const float* getColor();
        void setColor( float r, float g, float b, float a );
        
        #define TEXT_MAXWIDTH_NONE      -1
        // #define TEXT_MAXHEIGHT_ONELINE   0                                      // Doesn't work like it should, maybe?
        
        std::pair< int, int > getMaxDimensions();
        void                  setMaxDimensions( int w,
                                                int h,
                                                ellipsis_mode e = NONE );       // If w = -1 e is ignored
        
        bool getHinting();
        void setHinting( bool h );
        
        bool getAntialiasing();
        void setAntialiasing( bool h );
        
        void draw();
    };
}

/******************************************************************************//******************************************************************************/

#endif


