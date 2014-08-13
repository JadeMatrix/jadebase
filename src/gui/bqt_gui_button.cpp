/* 
 * bqt_gui_button.cpp
 * 
 * Implements GUI buttons
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_button.hpp"

#include "../bqt_mutex.hpp"
#include "../bqt_exception.hpp"
#include "bqt_gui_resource.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex button_rsrc_mutex;
    int button_count = 0;
    
    enum button_rsrc_pos
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER_CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
        
        NUM_POS
    };
    bqt::gui_resource* button_resources;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    button::button( layout& parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : layout_element( parent, x, y, w, h )
    {
        type = BLANK;
        auto_resize = false;
        togglable = false;
        state = UP_OFF;
        
        scoped_lock slock( button_rsrc_mutex );
        
        if( button_count < 1 )
        {
            if( button_count < 0 )
                throw exception( "button::button(): Button count < 0" );
            
            // for( int i = 0; i < NUM_POS; ++i )
                
        }
        
        ++button_count;
    }
    button::~button()
    {
        scoped_lock slock( button_rsrc_mutex );
        
        --button_count;
        
        if( button_count < 1 )
        {
            if( button_count < 0 )
                throw exception( "button::~button(): Button count < 0" );
            
            // release resources
        }
    }
    
    std::pair< unsigned int, unsigned int > button::getRealDimensions()
    {
        scoped_lock slock( element_mutex );
        
        if( auto_resize )
        {
            // calculate new dimensions
        }
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    // std::pair< unsigned int, unsigned int > button::getVisualDimensions()
    // {
    //     return getRealDimensions();
    // }
    std::pair< unsigned int, unsigned int > button::getMinDimensions()
    {
        // TODO: calculate
        return std::pair< unsigned int, unsigned int >( 12, 14 );
    }
    
    bool button::acceptEvent( window_event&e )
    {
        scoped_lock slock( element_mutex );
        
        return false;
    }
    
    void button::draw()
    {
        scoped_lock slock( element_mutex );
        
        std::pair< unsigned int, unsigned int > real_dimensions = getRealDimensions();
        
        
    }
    
    /* button-specific ****************************************************//******************************************************************************/
    
    void button::setText ( std::string t, bool c )
    {
        scoped_lock slock( element_mutex );
        
        str_contents = t;
        
        if( c )
            type = STRING;
    }
    // void button::setImage( ???? i, bool );
    void button::setImage( std::string f, bool c )
    {
        scoped_lock slock( element_mutex );
        
        throw exception( "button::setImage( std::string, bool ): Not implemented yet" );
        // TODO: implement
        
        if( c )
            type = IMAGE;
    }
    
    void button::clearContents( bool c )
    {
        scoped_lock slock( element_mutex );
        
        str_contents = "";
        // img_contents
        
        if( c )
            type = BLANK;
    }
    
    void button::setAutoResize( bool a )
    {
        scoped_lock slock( element_mutex );
        
        auto_resize = a;
    }
    bool button::getAutoResize()
    {
        scoped_lock slock( element_mutex );
        
        return auto_resize;
    }
    
    void button::setTogglable( bool t )
    {
        scoped_lock slock( element_mutex );
        
        togglable = t;
    }
    bool button::getTogglable()
    {
        scoped_lock slock( element_mutex );
        
        return togglable;
    }
}


