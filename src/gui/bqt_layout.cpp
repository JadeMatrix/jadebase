/* 
 * bqt_layout.cpp
 * 
 * Implements layout handlers from bqt_layout.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_layout.hpp"

#include "../bqt_exception.hpp"
#include "../bqt_gl.hpp"
#include "../bqt_log.hpp"
#include "../bqt_launchargs.hpp"
#include "../bqt_platform.h"
#include "../bqt_window.hpp"
#include "../bqt_taskexec.hpp"
#include "bqt_layout_element.hpp"
#include "bqt_gui_resource.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    /* LAYOUT *****************************************************************//******************************************************************************/
    
    #ifdef DEBUG
    #define GUI_RSRC_FILENAME "make/BQTDraw/Resources/gui_resources.png"
    #else
    #define GUI_RSRC_FILENAME "Resources/gui_resources.png"
    #endif
    
    void layout::initNamedResources()
    {
        named_resources[ rounded_button_off_up_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 0, 6, 7 );
        named_resources[ rounded_button_off_up_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 0, 1, 7 );
        named_resources[ rounded_button_off_up_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 0, 6, 7 );
        named_resources[ rounded_button_off_up_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 7, 6, 1 );
        named_resources[ rounded_button_off_up_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 7, 1, 1 );
        named_resources[ rounded_button_off_up_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 7, 6, 1 );
        named_resources[ rounded_button_off_up_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 8, 6, 7 );
        named_resources[ rounded_button_off_up_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 8, 1, 7 );
        named_resources[ rounded_button_off_up_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 8, 6, 7 );
        named_resources[ rounded_button_off_down_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 15, 6, 7 );
        named_resources[ rounded_button_off_down_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 15, 1, 7 );
        named_resources[ rounded_button_off_down_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 15, 6, 7 );
        named_resources[ rounded_button_off_down_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 22, 6, 1 );
        named_resources[ rounded_button_off_down_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 22, 1, 1 );
        named_resources[ rounded_button_off_down_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 22, 6, 1 );
        named_resources[ rounded_button_off_down_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 23, 6, 7 );
        named_resources[ rounded_button_off_down_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 23, 1, 7 );
        named_resources[ rounded_button_off_down_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 23, 6, 7 );
        named_resources[ rounded_button_on_up_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 30, 6, 7 );
        named_resources[ rounded_button_on_up_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 30, 1, 7 );
        named_resources[ rounded_button_on_up_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 30, 6, 7 );
        named_resources[ rounded_button_on_up_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 37, 6, 1 );
        named_resources[ rounded_button_on_up_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 37, 1, 1 );
        named_resources[ rounded_button_on_up_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 37, 6, 1 );
        named_resources[ rounded_button_on_up_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 38, 6, 7 );
        named_resources[ rounded_button_on_up_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 38, 1, 7 );
        named_resources[ rounded_button_on_up_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 38, 6, 7 );
        named_resources[ rounded_button_on_down_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 45, 6, 7 );
        named_resources[ rounded_button_on_down_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 45, 1, 7 );
        named_resources[ rounded_button_on_down_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 45, 6, 7 );
        named_resources[ rounded_button_on_down_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 52, 6, 1 );
        named_resources[ rounded_button_on_down_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 52, 1, 1 );
        named_resources[ rounded_button_on_down_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 52, 6, 1 );
        named_resources[ rounded_button_on_down_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 53, 6, 7 );
        named_resources[ rounded_button_on_down_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 53, 1, 7 );
        named_resources[ rounded_button_on_down_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 53, 6, 7 );
        named_resources[ squared_button_off_up_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 0, 6, 7 );
        named_resources[ squared_button_off_up_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 0, 1, 7 );
        named_resources[ squared_button_off_up_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 0, 6, 7 );
        named_resources[ squared_button_off_up_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 7, 6, 1 );
        named_resources[ squared_button_off_up_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 7, 1, 1 );
        named_resources[ squared_button_off_up_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 7, 6, 1 );
        named_resources[ squared_button_off_up_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 8, 6, 7 );
        named_resources[ squared_button_off_up_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 8, 1, 7 );
        named_resources[ squared_button_off_up_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 8, 6, 7 );
        named_resources[ squared_button_off_down_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 15, 6, 7 );
        named_resources[ squared_button_off_down_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 15, 1, 7 );
        named_resources[ squared_button_off_down_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 15, 6, 7 );
        named_resources[ squared_button_off_down_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 22, 6, 1 );
        named_resources[ squared_button_off_down_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 22, 1, 1 );
        named_resources[ squared_button_off_down_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 22, 6, 1 );
        named_resources[ squared_button_off_down_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 23, 6, 7 );
        named_resources[ squared_button_off_down_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 23, 1, 7 );
        named_resources[ squared_button_off_down_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 23, 6, 7 );
        named_resources[ squared_button_on_up_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 30, 6, 7 );
        named_resources[ squared_button_on_up_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 30, 1, 7 );
        named_resources[ squared_button_on_up_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 30, 6, 7 );
        named_resources[ squared_button_on_up_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 37, 6, 1 );
        named_resources[ squared_button_on_up_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 37, 1, 1 );
        named_resources[ squared_button_on_up_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 37, 6, 1 );
        named_resources[ squared_button_on_up_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 38, 6, 7 );
        named_resources[ squared_button_on_up_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 38, 1, 7 );
        named_resources[ squared_button_on_up_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 38, 6, 7 );
        named_resources[ squared_button_on_down_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 45, 6, 7 );
        named_resources[ squared_button_on_down_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 45, 1, 7 );
        named_resources[ squared_button_on_down_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 45, 6, 7 );
        named_resources[ squared_button_on_down_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 52, 6, 1 );
        named_resources[ squared_button_on_down_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 52, 1, 1 );
        named_resources[ squared_button_on_down_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 52, 6, 1 );
        named_resources[ squared_button_on_down_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 53, 6, 7 );
        named_resources[ squared_button_on_down_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 53, 1, 7 );
        named_resources[ squared_button_on_down_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 53, 6, 7 );
        named_resources[ tab_active_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 60, 6, 30 );
        named_resources[ tab_active_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 6, 60, 1, 30 );
        named_resources[ tab_active_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 7, 60, 6, 30 );
        named_resources[ tab_inactive_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 13, 60, 6, 30 );
        named_resources[ tab_inactive_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 19, 60, 1, 30 );
        named_resources[ tab_inactive_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 20, 60, 6, 30 );
        named_resources[ tab_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 26, 60, 1, 30 );
        named_resources[ tab_control_active_close_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 38, 110, 20, 23 );
        named_resources[ tab_control_active_close_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 58, 110, 20, 23 );
        named_resources[ tab_control_active_close_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 78, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 98, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 118, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 128, 110, 20, 23 );
        named_resources[ tab_control_inactive_close_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 38, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 58, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 78, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 98, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 118, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 128, 133, 20, 23 );
        named_resources[ toolbox_close_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 26, 0, 17, 17 );
        named_resources[ toolbox_close_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 26, 17, 17, 17 );
        named_resources[ toolbox_close_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 26, 34, 17, 17 );
        named_resources[ toolbox_collapse_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 43, 0, 17, 17 );
        named_resources[ toolbox_collapse_over ] = new gui_resource( *this, GUI_RSRC_FILENAME, 43, 17, 17, 17 );
        named_resources[ toolbox_collapse_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 43, 34, 17, 17 );
        named_resources[ toolbox_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 27, 51, 1, 17 );
        named_resources[ ruler_corner_below ] = new gui_resource( *this, GUI_RSRC_FILENAME, 28, 51, 16, 16 );
        named_resources[ ruler_corner_above ] = new gui_resource( *this, GUI_RSRC_FILENAME, 44, 51, 16, 16 );
        named_resources[ ruler_mark_inside_large ] = new gui_resource( *this, GUI_RSRC_FILENAME, 63, 51, 1, 15 );
        named_resources[ ruler_mark_inside_medium ] = new gui_resource( *this, GUI_RSRC_FILENAME, 64, 51, 1, 15 );
        named_resources[ ruler_mark_inside_small ] = new gui_resource( *this, GUI_RSRC_FILENAME, 65, 51, 1, 15 );
        named_resources[ ruler_mark_outside_large ] = new gui_resource( *this, GUI_RSRC_FILENAME, 60, 51, 1, 15 );
        named_resources[ ruler_mark_outside_medium ] = new gui_resource( *this, GUI_RSRC_FILENAME, 61, 51, 1, 15 );
        named_resources[ ruler_mark_outside_small ] = new gui_resource( *this, GUI_RSRC_FILENAME, 62, 51, 1, 15 );
        named_resources[ ruler_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 66, 51, 1, 15 );
        named_resources[ divider ] = new gui_resource( *this, GUI_RSRC_FILENAME, 60, 42, 1, 1 );
        named_resources[ scrollbar_button_left_bottom_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 27, 68, 19, 10 );
        named_resources[ scrollbar_button_left_bottom_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 27, 78, 19, 10 );
        named_resources[ scrollbar_button_right_top_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 58, 68, 19, 10 );
        named_resources[ scrollbar_button_right_top_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 46, 78, 19, 10 );
        named_resources[ scrollbar_bar_left_bottom ] = new gui_resource( *this, GUI_RSRC_FILENAME, 47, 68, 5, 10 );
        named_resources[ scrollbar_bar_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 52, 68, 1, 10 );
        named_resources[ scrollbar_bar_right_top ] = new gui_resource( *this, GUI_RSRC_FILENAME, 53, 68, 5, 10 );
        named_resources[ scrollbar_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 46, 68, 1, 10 );
        named_resources[ scrollbar_corner ] = new gui_resource( *this, GUI_RSRC_FILENAME, 77, 68, 10, 10 );
        named_resources[ viewbar_handle ] = new gui_resource( *this, GUI_RSRC_FILENAME, 88, 68, 20, 8 );
        named_resources[ viewbar_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 87, 68, 1, 8 );
        named_resources[ viewbar_corner ] = new gui_resource( *this, GUI_RSRC_FILENAME, 108, 68, 8, 8 );
        named_resources[ dropdown_button_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 67, 20, 21, 22 );
        named_resources[ dropdown_button_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 88, 20, 21, 22 );
        named_resources[ dropdown_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 60, 20, 6, 22 );
        named_resources[ dropdown_fill ] = new gui_resource( *this, GUI_RSRC_FILENAME, 66, 20, 1, 22 );
        named_resources[ radio_off_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 60, 0, 14, 20 );
        named_resources[ radio_off_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 74, 0, 14, 20 );
        named_resources[ radio_on_up ] = new gui_resource( *this, GUI_RSRC_FILENAME, 88, 0, 14, 20 );
        named_resources[ radio_on_down ] = new gui_resource( *this, GUI_RSRC_FILENAME, 102, 0, 14, 20 );
        named_resources[ buttonmenu_top_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 90, 22, 15 );
        named_resources[ buttonmenu_top_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 22, 90, 1, 15 );
        named_resources[ buttonmenu_top_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 23, 90, 15, 15 );
        named_resources[ buttonmenu_center_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 105, 22, 1 );
        named_resources[ buttonmenu_center_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 22, 105, 1, 1 );
        named_resources[ buttonmenu_center_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 23, 105, 15, 1 );
        named_resources[ buttonmenu_bottom_left ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 141, 22, 15 );
        named_resources[ buttonmenu_bottom_center ] = new gui_resource( *this, GUI_RSRC_FILENAME, 22, 141, 1, 15 );
        named_resources[ buttonmenu_bottom_right ] = new gui_resource( *this, GUI_RSRC_FILENAME, 23, 141, 15, 15 );
        named_resources[ buttonmenu_arrow ] = new gui_resource( *this, GUI_RSRC_FILENAME, 0, 106, 22, 35 );
    }
    // void layout::deinitNamedResources()
    // {
    //     for( std::map< gui_resource_name, gui_resource* >::iterator iter = named_resources.begin();
    //          iter != named_resources.end();
    //          ++iter )
    //     {
    //         delete iter -> second;
    //     }
        
    //     named_resources.clear();
    // }
    
    layout::layout( window* p, unsigned int w, unsigned int h )
    {
        parent = p;
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
    }
    
    std::pair< unsigned int, unsigned int > layout::getDimensions()
    {
        scoped_lock slock( layout_mutex );
        
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    void layout::setDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock slock( layout_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = w;
    }
    
    gui_texture* layout::acquireTexture( std::string f )
    {
        scoped_lock slock( layout_mutex );
        
        gui_texture_wrapper* w;
        
        if( resource_textures.count( f ) )
        {
            w = resource_textures[ f ];
            
            ff::write( bqt_out,
                       "Acquiring existing texture \"",
                       f,
                       "\" as 0x",
                       ff::to_x( ( unsigned long )( w -> texture ) ),
                       "\n" );
        }
        else
        {
            w = new gui_texture_wrapper();
            resource_textures[ f ] = w;
            
            submitTask( new InitLayoutResources_task( *this ) );
            
            ff::write( bqt_out,
                       "Acquiring new texture \"",
                       f,
                       "\" as 0x",
                       ff::to_x( ( unsigned long )( w -> texture ) ),
                       "\n" );
        }
        
        w -> ref_count++;
        
        return w -> texture;
    }
    void layout::releaseTexture( gui_texture* t )
    {
        scoped_lock slock( layout_mutex );
        
        for( std::map< std::string, gui_texture_wrapper* >::iterator iter = resource_textures.begin();
             iter != resource_textures.end();
             ++iter )
         {
            if( iter -> second -> texture == t )
            {
                iter -> second -> ref_count--;
                
                ff::write( bqt_out,
                           "Releasing texture 0x",
                           ff::to_x( ( unsigned long )( iter -> second -> texture ) ),
                           ", ref count is now ",
                           iter -> second -> ref_count,
                           "\n" );
                
                submitTask( new DeinitLayoutResources_task( *this ) );
                
                return;
            }
         }
         
         throw exception( "layout::releaseTexture(): Texture not in layout" );
    }
    
    gui_resource* layout::getNamedResource( gui_resource_name name )
    {
        if( named_resources.count( name ) )
            return named_resources[ name ];
        else
            throw exception( "layout::getNamedResource(): Unknown name" );
    }
    
    // bool layout::addNamedResource( gui_resource_name n, gui_resource* r )
    // {
    //     if( named_resources.count( n ) )
    //         return false;
    //     else
    //     {
    //         named_resources[ n ] = r;
    //         new_resources = true;
    //         return true;
    //     }
    // }
    
    void layout::acceptEvent( window_event& e )
    {
        scoped_lock slock( layout_mutex );
        
        bool pass_event = false;
        
        ff::write( bqt_out, "Got event: ", wevent2str( e ), "\n" );
        
        switch( e.type )
        {
        case STROKE:
        case PINCH:
            // {
            //     if( input_assoc.count( e.stroke.dev_id ) )
            //     {
            //         if( ( e.type == STROKE && e.stroke.pressure <= 0.0f )
            //             || ( e.type == PINCH && e.pinch.finish ) )              // If lifted, de-associate
            //             input_assoc.erase( e.stroke.dev_id );
                    
            //         input_assoc[ e.stroke.dev_id ] -> acceptEvent( e );         // This works because elements store their position
            //     }
            //     else
            //         pass_event = true;
            // }
            break;
        case DROP:
        case KEYCOMMAND:
            if( e.key.key == KEY_Q && e.key.cmd )
            {
                setQuitFlag();
                break;
            }
        case COMMAND:
            pass_event = true;
            break;
        case TEXT:
            // {
            //     if( e.text.element == NULL )
            //         throw exception( "layout::acceptEvent(): Text input event with no associated element" );
                
            //     e.text.element -> acceptEvent( e );
            // }
            break;
        default:
            throw exception( "layout::acceptEvent(): Uknown event type" );
            break;
        }
        
        if( pass_event )
        {
            layout_element* acceptor = NULL;
            
            for( int i = elements.size() - 1; i >= 0; --i )                     // Iterate backwards as last element is topmost
            {
                float position[ 2 ];
                bool no_position = false;
                
                switch( e.type )
                {
                case STROKE:
                    position[ 0 ] = e.stroke.position[ 0 ];
                    position[ 1 ] = e.stroke.position[ 1 ];
                    break;
                case PINCH:
                    position[ 0 ] = e.pinch.position[ 0 ];
                    position[ 1 ] = e.pinch.position[ 1 ];
                    break;
                case DROP:
                    position[ 0 ] = e.drop.position[ 0 ];
                    position[ 1 ] = e.drop.position[ 1 ];
                    break;
                case KEYCOMMAND:
                case COMMAND:
                    no_position = true;
                    break;
                default:
                    break;                                                      // Any other types should have already been handled above
                }
                
                if( elements[ i ] -> acceptEvent( e ) )
                    break;
                
                // if( no_position )
                // {
                //     acceptor = elements[ i ] -> acceptEvent( e );
                    
                //     if( acceptor != NULL )                                      // layout_element::acceptEvent() must check fallthrough as layout_elements may
                //                                                                 // have sub elements.
                //         break;
                // }
                // else
                // {
                //     std::pair< int, int > element_pos = elements[ i ] -> getPosition();
                //     std::pair< unsigned int, unsigned int > element_dim = elements[ i ] -> getDimensions();
                    
                //     if(    position[ 0 ] <= element_pos.first
                //         && position[ 0 ] >  element_pos.first  + element_dim.first
                //         && position[ 1 ] <= element_pos.second
                //         && position[ 1 ] >  element_pos.second + element_dim.second )
                //     {
                //         acceptor = elements[ i ] -> acceptEvent( e );
                        
                //         if( acceptor != NULL )
                //         {
                //             if( ( e.type == STROKE && e.stroke.pressure > 0.0f )// Only track events with pressure (ie not hovering)
                //                 || ( e.type == PINCH && !e.pinch.finish ) )     // Also, no tracking drop events even though they have position
                //                 input_assoc[ e.stroke.dev_id ] = acceptor;
                            
                //             break;
                //         }
                //     }
                // }
            }
            
            if( acceptor == NULL && getDevMode() )
                ff::write( bqt_out,
                           "Warning: event trickled down but not accepted\n",
                           " - Event: ",
                           wevent2str( e ),
                           "\n" );
        }
    }
    
    void layout::draw()
    {
        scoped_lock slock( layout_mutex );
        
        for( int i = 0; i < elements.size(); ++i )                              // Iterate forwards as last element is topmost
        {
            std::pair< int, int > pos = elements[ i ] -> getPosition();
            
            glTranslatef( pos.first,
                          pos.second,
                          0.0f );
            
            elements[ i ] -> draw();
            
            glTranslatef( -1 * pos.first,
                          -1 * pos.second,
                          0.0f );
        }
    }
    
    void layout::startClean()
    {
        ff::write( bqt_out,
                   "Starting clean:\n",
                   "    elements: ",
                   elements.size(),
                   "\n    named_resources: ",
                   named_resources.size(),
                   "\n" );
        
        scoped_lock slock( layout_mutex );
        
        for( int i = 0; i < elements.size(); ++i )                              // Iterate forwards as last element is topmost
        {
            delete elements[ i ];
        }
        elements.clear();
        
        for( std::map< gui_resource_name, gui_resource* >::iterator iter = named_resources.begin();
             iter != named_resources.end();
             ++iter )
        {
            delete iter -> second;
        }
        named_resources.clear();
        
        submitTask( new DeinitLayoutResources_task( *this ) );
    }
    bool layout::isClean()
    {
        scoped_lock slock( layout_mutex );
        
        return !resource_textures.size();
    }
    
    /* INITLAYOUTRESOURCES_TASK ***********************************************//******************************************************************************/
    
    layout::InitLayoutResources_task::InitLayoutResources_task( layout& t ) : target( t )
    {
        state = OPENING_FILES;
    }
    
    bool layout::InitLayoutResources_task::execute( task_mask* caller_mask )
    {
        scoped_lock slock( target.layout_mutex );
        
        if( state == UPLOADING_TEXTURES )
            target.parent -> makeContextCurrent();
        
        for( std::map< std::string, gui_texture_wrapper* >::iterator iter = target.resource_textures.begin();
             iter != target.resource_textures.end();
             ++iter )
        {
            switch( state )
            {
            case OPENING_FILES:
                {
                    if( iter -> second -> texture -> gl_texture == 0x00
                        && iter -> second -> data == NULL )
                    {
                        ff::write( bqt_out,
                                   "Opening \"",
                                   iter -> first,
                                   "\" as a resource texture\n" );
                        
                        bqt::png_file rsrc_file( iter -> first );
                        
                        std::pair< unsigned int, unsigned int > rsrc_dim = rsrc_file.getDimensions();
                        
                        iter -> second -> data = new unsigned char[ rsrc_dim.first * rsrc_dim.second * 4 ];
                        
                        if( iter -> second -> data == NULL )
                            throw bqt::exception( "InitLayout_task::execute(): Could not allocate swap space" );
                        
                        rsrc_file.toRGBABytes( iter -> second -> data );
                        
                        iter -> second -> texture -> dimensions[ 0 ] = rsrc_dim.first;
                        iter -> second -> texture -> dimensions[ 1 ] = rsrc_dim.second;
                    }
                }
                break;
            case UPLOADING_TEXTURES:
                {
                    if( !( *caller_mask & TASK_GPU ) )
                        throw exception( "not on task thread" );
                    
                    if( iter -> second -> texture -> gl_texture == 0x00
                        && iter -> second -> data != NULL )
                    {
                        glGenTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                        
                        if( iter -> second -> texture -> gl_texture == 0x00 )
                            throw exception( "InitLayoutResources_task::execute(): Could not generate texture" );
                        
                        ff::write( bqt_out,
                                   "Uploading \"",
                                   iter -> first,
                                   "\" as a resource texture 0x",
                                   ff::to_x( iter -> second -> texture -> gl_texture ),
                                   "\n" );
                        
                        glBindTexture( GL_TEXTURE_2D, iter -> second -> texture -> gl_texture );
                        glTexImage2D( GL_TEXTURE_2D,
                                      0,
                                      GL_RGBA,
                                      iter -> second -> texture -> dimensions[ 1 ],
                                      iter -> second -> texture -> dimensions[ 0 ],
                                      0,
                                      GL_RGBA,
                                      GL_UNSIGNED_BYTE,
                                      iter -> second -> data );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                        
                        glBindTexture( GL_TEXTURE_2D, 0 );
                        
                        GLenum gl_error = glGetError();
                        if( gl_error != GL_NO_ERROR )
                        {
                            bqt::exception e;
                            ff::write( *e,
                                       "LoadGUIResource_task::execute(): OpenGL error: ",
                                       "...",
                                       " loading pixels from 0x",
                                       ff::to_x( ( unsigned long )( iter -> second -> data ), HEX_WIDTH, HEX_WIDTH ),
                                       " to texture 0x",
                                       ff::to_x( iter -> second -> texture -> gl_texture, HEX_WIDTH, HEX_WIDTH ) );
                            throw e;
                        }
                        
                        delete[] iter -> second -> data;
                        iter -> second -> data = NULL;
                    }
                    
                    if( iter -> second -> texture -> gl_texture == 0x00
                        && iter -> second -> data == NULL )
                    {
                        throw exception( "still no data" );
                    }
                }
                break;
            default:
                throw exception( "InitLayoutResources_task::execute(): Unkown state" );
            }
        }
        
        if( state == OPENING_FILES )
        {
            state = UPLOADING_TEXTURES;
            return false;                                                       // Requeue for next pass
        }
        else
            return true;
    }
    task_mask layout::InitLayoutResources_task::getMask()
    {
        switch( state )
        {
        case OPENING_FILES:
            return TASK_SYSTEM;
        case UPLOADING_TEXTURES:
            return TASK_GPU;
        default:
            throw exception( "InitLayout_task::getMask(): Unkown state" );
        }
    }
    
    /* DEINITLAYOUTRESOURCES_TASK *********************************************//******************************************************************************/
    
    layout::DeinitLayoutResources_task::DeinitLayoutResources_task( layout& t ) : target( t )
    {
        
    }
    
    bool layout::DeinitLayoutResources_task::execute( task_mask* caller_mask )
    {
        scoped_lock slock( target.layout_mutex );
        
        target.parent -> makeContextCurrent();
        
        for( std::map< std::string, gui_texture_wrapper* >::iterator iter = target.resource_textures.begin();
             iter != target.resource_textures.end(); /* NULL */ )
        {
            if( iter -> second -> ref_count < 1 )
            {
                ff::write( bqt_out,
                           "Deleting \"",
                           iter -> first,
                           "\" as a resource texture 0x",
                           ff::to_x( iter -> second -> texture -> gl_texture ),
                           "\n" );
                
                if( iter -> second -> texture -> gl_texture != 0x00 )
                    glDeleteTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                
                delete iter -> second;
                
                target.resource_textures.erase( iter++ );
            }
            else
                ++iter;
        }
        
        return true;
    }
    task_mask layout::DeinitLayoutResources_task::getMask()
    {
        return TASK_GPU;
    }
}


