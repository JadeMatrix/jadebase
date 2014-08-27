/* 
 * bqt_named_resources.cpp
 * 
 * Implements bqt_named_resources.hpp as well as acquireTexture() and
 * releaseTextre() from bqt_gui_texture.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_named_resources.hpp"

#include "../threading/bqt_rwlock.hpp"
#include "../bqt_task.hpp"
#include "../bqt_taskexec.hpp"
#include "bqt_gui_texture.hpp"
#include "../bqt_png.hpp"
#include "bqt_gui_image_rsrc.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_gl.hpp"
#include "../bqt_windowmanagement.hpp"
#include "../bqt_launchargs.hpp"
#include "../bqt_log.hpp"

/******************************************************************************//******************************************************************************/

#if DEBUG
#define GUI_RESOURCE_FILE "make/BQTDraw/Resources/gui_resources.png"
#else
#define GUI_RESOURCE_FILE "Resources/gui_resources.png"
#endif

namespace
{
    bqt::rwlock resources_lock;
    
    struct gui_texture_holder
    {
        bqt::gui_texture* texture;
        
        unsigned char* data;
        int ref_count;
        
        gui_texture_holder()
        {
            texture = new bqt::gui_texture();
            data = NULL;
            ref_count = 0;
        }
        ~gui_texture_holder()
        {
            delete texture;
        }
    };
    
    std::map< std::string, gui_texture_holder* > resource_textures;
    bool new_textures = false;
    bool old_textures = false;
    
    std::map< bqt::gui_resource_name, bqt::gui_resource* > named_resources;
    
    class ManageResources_task : public bqt::task
    {
    protected:
        bqt::task_mask current_mask;
        bool cleanup;
    public:
        ManageResources_task( bool c )
        {
            current_mask = bqt::TASK_SYSTEM;
            cleanup = c;
        }
        
        bool execute( bqt::task_mask* caller_mask )
        {
            bqt::scoped_lock< bqt::rwlock > slock( resources_lock, RW_WRITE );
            
            if( current_mask == bqt::TASK_SYSTEM )
            {
                if( !cleanup && bqt::getActiveWindow() == NULL )
                {
                    if( bqt::getDevMode() )
                        ff::write( bqt_out, "No active window, requeuing resource management\n" );
                    
                    return false;
                }
                
                if( new_textures )
                {
                    for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                         iter != resource_textures.end();
                         ++iter )
                    {
                        if( iter -> second -> data == NULL
                            && iter -> second -> texture -> gl_texture == 0x00 )
                        {
                            bqt::png_file rsrc_file( iter -> first );
                            
                            std::pair< unsigned int, unsigned int > rsrc_dim = rsrc_file.getDimensions();
                            
                            iter -> second -> data = new unsigned char[ rsrc_dim.first * rsrc_dim.second * 4 ];
                            
                            if( iter -> second -> data == NULL )
                                throw bqt::exception( "ManageResources_task::execute(): Could not allocate conversion space" );
                            
                            rsrc_file.toRGBABytes( iter -> second -> data );
                            
                            iter -> second -> texture -> dimensions[ 0 ] = rsrc_dim.first;
                            iter -> second -> texture -> dimensions[ 1 ] = rsrc_dim.second;
                        }
                    }
                }
                
                current_mask = bqt::TASK_GPU;
                
                return false;
            }
            else
            {
                if( new_textures )
                {
                    for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                         iter != resource_textures.end();
                         ++iter )
                    {
                        if( iter -> second -> texture -> gl_texture == 0x00
                            && iter -> second -> data != NULL )
                        {
                            GLuint gl_texture = bqt::bytesToTexture( iter -> second -> data,
                                                                     iter -> second -> texture -> dimensions[ 0 ],
                                                                     iter -> second -> texture -> dimensions[ 1 ] );
                            
                            iter -> second -> texture -> gl_texture = gl_texture;
                            
                            delete[] iter -> second -> data;
                            iter -> second -> data = NULL;
                        }
                    }
                    
                    new_textures = false;
                }
                
                if( old_textures )
                {
                    for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                         iter != resource_textures.end();
                         /* NULL */ )
                    {
                        if( iter -> second -> ref_count < 1 )
                        {
                            if( iter -> second -> texture -> gl_texture != 0x00 )
                                glDeleteTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                            
                            delete iter -> second;
                            
                            resource_textures.erase( iter++ );
                        }
                        else
                            ++iter;
                    }
                    
                    old_textures = false;
                }
                
                bqt::redrawAllWindows();
                
                return true;
            }
        }
        
        bqt::task_priority getPriority()
        {
            if( cleanup )
                return bqt::PRIORITY_HIGH;
            else
                return bqt::PRIORITY_NONE;
        }
        
        bqt::task_mask getMask()
        {
            return current_mask;
        }
    };
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void initNamedResources()
    {
        scoped_lock< rwlock > slock( resources_lock, RW_WRITE );
        
        named_resources[ rounded_button_off_up_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 0, 6, 7 );
        named_resources[ rounded_button_off_up_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 0, 1, 7 );
        named_resources[ rounded_button_off_up_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 0, 6, 7 );
        named_resources[ rounded_button_off_up_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 7, 6, 1 );
        named_resources[ rounded_button_off_up_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 7, 1, 1 );
        named_resources[ rounded_button_off_up_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 7, 6, 1 );
        named_resources[ rounded_button_off_up_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 8, 6, 7 );
        named_resources[ rounded_button_off_up_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 8, 1, 7 );
        named_resources[ rounded_button_off_up_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 8, 6, 7 );
        named_resources[ rounded_button_off_down_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 15, 6, 7 );
        named_resources[ rounded_button_off_down_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 15, 1, 7 );
        named_resources[ rounded_button_off_down_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 15, 6, 7 );
        named_resources[ rounded_button_off_down_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 22, 6, 1 );
        named_resources[ rounded_button_off_down_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 22, 1, 1 );
        named_resources[ rounded_button_off_down_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 22, 6, 1 );
        named_resources[ rounded_button_off_down_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 23, 6, 7 );
        named_resources[ rounded_button_off_down_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 23, 1, 7 );
        named_resources[ rounded_button_off_down_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 23, 6, 7 );
        named_resources[ rounded_button_on_up_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 30, 6, 7 );
        named_resources[ rounded_button_on_up_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 30, 1, 7 );
        named_resources[ rounded_button_on_up_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 30, 6, 7 );
        named_resources[ rounded_button_on_up_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 37, 6, 1 );
        named_resources[ rounded_button_on_up_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 37, 1, 1 );
        named_resources[ rounded_button_on_up_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 37, 6, 1 );
        named_resources[ rounded_button_on_up_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 38, 6, 7 );
        named_resources[ rounded_button_on_up_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 38, 1, 7 );
        named_resources[ rounded_button_on_up_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 38, 6, 7 );
        named_resources[ rounded_button_on_down_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 45, 6, 7 );
        named_resources[ rounded_button_on_down_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 45, 1, 7 );
        named_resources[ rounded_button_on_down_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 45, 6, 7 );
        named_resources[ rounded_button_on_down_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 52, 6, 1 );
        named_resources[ rounded_button_on_down_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 52, 1, 1 );
        named_resources[ rounded_button_on_down_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 52, 6, 1 );
        named_resources[ rounded_button_on_down_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 53, 6, 7 );
        named_resources[ rounded_button_on_down_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 53, 1, 7 );
        named_resources[ rounded_button_on_down_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 53, 6, 7 );
        named_resources[ squared_button_off_up_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 0, 6, 7 );
        named_resources[ squared_button_off_up_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 0, 1, 7 );
        named_resources[ squared_button_off_up_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 0, 6, 7 );
        named_resources[ squared_button_off_up_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 7, 6, 1 );
        named_resources[ squared_button_off_up_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 7, 1, 1 );
        named_resources[ squared_button_off_up_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 7, 6, 1 );
        named_resources[ squared_button_off_up_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 8, 6, 7 );
        named_resources[ squared_button_off_up_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 8, 1, 7 );
        named_resources[ squared_button_off_up_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 8, 6, 7 );
        named_resources[ squared_button_off_down_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 15, 6, 7 );
        named_resources[ squared_button_off_down_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 15, 1, 7 );
        named_resources[ squared_button_off_down_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 15, 6, 7 );
        named_resources[ squared_button_off_down_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 22, 6, 1 );
        named_resources[ squared_button_off_down_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 22, 1, 1 );
        named_resources[ squared_button_off_down_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 22, 6, 1 );
        named_resources[ squared_button_off_down_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 23, 6, 7 );
        named_resources[ squared_button_off_down_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 23, 1, 7 );
        named_resources[ squared_button_off_down_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 23, 6, 7 );
        named_resources[ squared_button_on_up_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 30, 6, 7 );
        named_resources[ squared_button_on_up_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 30, 1, 7 );
        named_resources[ squared_button_on_up_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 30, 6, 7 );
        named_resources[ squared_button_on_up_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 37, 6, 1 );
        named_resources[ squared_button_on_up_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 37, 1, 1 );
        named_resources[ squared_button_on_up_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 37, 6, 1 );
        named_resources[ squared_button_on_up_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 38, 6, 7 );
        named_resources[ squared_button_on_up_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 38, 1, 7 );
        named_resources[ squared_button_on_up_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 38, 6, 7 );
        named_resources[ squared_button_on_down_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 45, 6, 7 );
        named_resources[ squared_button_on_down_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 45, 1, 7 );
        named_resources[ squared_button_on_down_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 45, 6, 7 );
        named_resources[ squared_button_on_down_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 52, 6, 1 );
        named_resources[ squared_button_on_down_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 52, 1, 1 );
        named_resources[ squared_button_on_down_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 52, 6, 1 );
        named_resources[ squared_button_on_down_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 53, 6, 7 );
        named_resources[ squared_button_on_down_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 19, 53, 1, 7 );
        named_resources[ squared_button_on_down_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 53, 6, 7 );
        named_resources[ tab_active_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 60, 6, 30 );
        named_resources[ tab_active_center ] = new image_rsrc( GUI_RESOURCE_FILE, 6, 60, 1, 30 );
        named_resources[ tab_active_right ] = new image_rsrc( GUI_RESOURCE_FILE, 7, 60, 6, 30 );
        named_resources[ tab_inactive_left ] = new image_rsrc( GUI_RESOURCE_FILE, 14, 60, 6, 30 );
        named_resources[ tab_inactive_center ] = new image_rsrc( GUI_RESOURCE_FILE, 20, 60, 1, 30 );
        named_resources[ tab_inactive_right ] = new image_rsrc( GUI_RESOURCE_FILE, 21, 60, 6, 30 );
        named_resources[ tab_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 13, 60, 1, 30 );
        named_resources[ tab_control_active_close_up ] = new image_rsrc( GUI_RESOURCE_FILE, 38, 110, 20, 23 );
        named_resources[ tab_control_active_close_over ] = new image_rsrc( GUI_RESOURCE_FILE, 58, 110, 20, 23 );
        named_resources[ tab_control_active_close_down ] = new image_rsrc( GUI_RESOURCE_FILE, 78, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_up ] = new image_rsrc( GUI_RESOURCE_FILE, 98, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_over ] = new image_rsrc( GUI_RESOURCE_FILE, 118, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_down ] = new image_rsrc( GUI_RESOURCE_FILE, 128, 110, 20, 23 );
        named_resources[ tab_control_inactive_close_up ] = new image_rsrc( GUI_RESOURCE_FILE, 38, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_over ] = new image_rsrc( GUI_RESOURCE_FILE, 58, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_down ] = new image_rsrc( GUI_RESOURCE_FILE, 78, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_up ] = new image_rsrc( GUI_RESOURCE_FILE, 98, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_over ] = new image_rsrc( GUI_RESOURCE_FILE, 118, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_down ] = new image_rsrc( GUI_RESOURCE_FILE, 128, 133, 20, 23 );
        named_resources[ toolbox_close_up ] = new image_rsrc( GUI_RESOURCE_FILE, 26, 0, 17, 17 );
        named_resources[ toolbox_close_over ] = new image_rsrc( GUI_RESOURCE_FILE, 26, 17, 17, 17 );
        named_resources[ toolbox_close_down ] = new image_rsrc( GUI_RESOURCE_FILE, 26, 34, 17, 17 );
        named_resources[ toolbox_collapse_up ] = new image_rsrc( GUI_RESOURCE_FILE, 43, 0, 17, 17 );
        named_resources[ toolbox_collapse_over ] = new image_rsrc( GUI_RESOURCE_FILE, 43, 17, 17, 17 );
        named_resources[ toolbox_collapse_down ] = new image_rsrc( GUI_RESOURCE_FILE, 43, 34, 17, 17 );
        named_resources[ toolbox_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 27, 51, 1, 17 );
        named_resources[ ruler_corner_below ] = new image_rsrc( GUI_RESOURCE_FILE, 28, 51, 16, 16 );
        named_resources[ ruler_corner_above ] = new image_rsrc( GUI_RESOURCE_FILE, 44, 51, 16, 16 );
        named_resources[ ruler_mark_inside_large ] = new image_rsrc( GUI_RESOURCE_FILE, 63, 51, 1, 15 );
        named_resources[ ruler_mark_inside_medium ] = new image_rsrc( GUI_RESOURCE_FILE, 64, 51, 1, 15 );
        named_resources[ ruler_mark_inside_small ] = new image_rsrc( GUI_RESOURCE_FILE, 65, 51, 1, 15 );
        named_resources[ ruler_mark_outside_large ] = new image_rsrc( GUI_RESOURCE_FILE, 60, 51, 1, 15 );
        named_resources[ ruler_mark_outside_medium ] = new image_rsrc( GUI_RESOURCE_FILE, 61, 51, 1, 15 );
        named_resources[ ruler_mark_outside_small ] = new image_rsrc( GUI_RESOURCE_FILE, 62, 51, 1, 15 );
        named_resources[ ruler_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 66, 51, 1, 15 );
        named_resources[ divider ] = new image_rsrc( GUI_RESOURCE_FILE, 60, 42, 1, 1 );
        named_resources[ scrollbar_button_left_bottom_up ] = new image_rsrc( GUI_RESOURCE_FILE, 189, 9, 25, 12 );
        named_resources[ scrollbar_button_left_bottom_down ] = new image_rsrc( GUI_RESOURCE_FILE, 189, 12, 25, 12 );
        named_resources[ scrollbar_button_right_top_up ] = new image_rsrc( GUI_RESOURCE_FILE, 150, 0, 25, 12 );
        named_resources[ scrollbar_button_right_top_down ] = new image_rsrc( GUI_RESOURCE_FILE, 150, 12, 25, 12 );
        named_resources[ scrollbar_bar_left_bottom_up ] = new image_rsrc( GUI_RESOURCE_FILE, 183, 0, 6, 12 );
        named_resources[ scrollbar_bar_center_up ] = new image_rsrc( GUI_RESOURCE_FILE, 182, 0, 1, 12 );
        named_resources[ scrollbar_bar_right_top_up ] = new image_rsrc( GUI_RESOURCE_FILE, 176, 0, 6, 12 );
        named_resources[ scrollbar_bar_left_bottom_down ] = new image_rsrc( GUI_RESOURCE_FILE, 183, 12, 6, 12 );
        named_resources[ scrollbar_bar_center_down ] = new image_rsrc( GUI_RESOURCE_FILE, 182, 12, 1, 12 );
        named_resources[ scrollbar_bar_right_top_down ] = new image_rsrc( GUI_RESOURCE_FILE, 176, 12, 6, 12 );
        named_resources[ scrollbar_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 175, 0, 1, 12 );
        named_resources[ scrollbar_corner_up ] = new image_rsrc( GUI_RESOURCE_FILE, 214, 0, 12, 12 );
        named_resources[ scrollbar_corner_down ] = new image_rsrc( GUI_RESOURCE_FILE, 214, 12, 12, 12 );
        named_resources[ scrollbar_corner_evil ] = new image_rsrc( GUI_RESOURCE_FILE, 226, 0, 12, 12 );
        named_resources[ viewbar_handle ] = new image_rsrc( GUI_RESOURCE_FILE, 87, 68, 20, 8 );
        named_resources[ viewbar_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 107, 68, 1, 8 );
        named_resources[ viewbar_corner ] = new image_rsrc( GUI_RESOURCE_FILE, 108, 68, 8, 8 );
        named_resources[ dropdown_button_up ] = new image_rsrc( GUI_RESOURCE_FILE, 67, 20, 21, 22 );
        named_resources[ dropdown_button_down ] = new image_rsrc( GUI_RESOURCE_FILE, 88, 20, 21, 22 );
        named_resources[ dropdown_left ] = new image_rsrc( GUI_RESOURCE_FILE, 60, 20, 6, 22 );
        named_resources[ dropdown_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 66, 20, 1, 22 );
        named_resources[ radio_off_up ] = new image_rsrc( GUI_RESOURCE_FILE, 60, 0, 14, 20 );
        named_resources[ radio_off_down ] = new image_rsrc( GUI_RESOURCE_FILE, 74, 0, 14, 20 );
        named_resources[ radio_on_up ] = new image_rsrc( GUI_RESOURCE_FILE, 88, 0, 14, 20 );
        named_resources[ radio_on_down ] = new image_rsrc( GUI_RESOURCE_FILE, 102, 0, 14, 20 );
        named_resources[ buttonmenu_top_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 90, 22, 15 );
        named_resources[ buttonmenu_top_center ] = new image_rsrc( GUI_RESOURCE_FILE, 22, 90, 1, 15 );
        named_resources[ buttonmenu_top_right ] = new image_rsrc( GUI_RESOURCE_FILE, 23, 90, 15, 15 );
        named_resources[ buttonmenu_center_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 105, 22, 1 );
        named_resources[ buttonmenu_center_center ] = new image_rsrc( GUI_RESOURCE_FILE, 22, 105, 1, 1 );
        named_resources[ buttonmenu_center_right ] = new image_rsrc( GUI_RESOURCE_FILE, 23, 105, 15, 1 );
        named_resources[ buttonmenu_bottom_left ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 141, 22, 15 );
        named_resources[ buttonmenu_bottom_center ] = new image_rsrc( GUI_RESOURCE_FILE, 22, 141, 1, 15 );
        named_resources[ buttonmenu_bottom_right ] = new image_rsrc( GUI_RESOURCE_FILE, 23, 141, 15, 15 );
        named_resources[ buttonmenu_arrow ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 106, 22, 35 );
        named_resources[ checkbox_off_up ] = new image_rsrc( GUI_RESOURCE_FILE, 116, 0, 17, 20 );
        named_resources[ checkbox_off_down ] = new image_rsrc( GUI_RESOURCE_FILE, 133, 0, 17, 20 );
        named_resources[ checkbox_part_up ] = new image_rsrc( GUI_RESOURCE_FILE, 116, 20, 17, 20 );
        named_resources[ checkbox_part_down ] = new image_rsrc( GUI_RESOURCE_FILE, 116, 40, 17, 20 );
        named_resources[ checkbox_on_up ] = new image_rsrc( GUI_RESOURCE_FILE, 133, 20, 17, 20 );
        named_resources[ checkbox_on_down ] = new image_rsrc( GUI_RESOURCE_FILE, 133, 40, 17, 20 );
        named_resources[ slider_empty_left ] = new image_rsrc( GUI_RESOURCE_FILE, 30, 44, 2, 4 );
        named_resources[ slider_empty_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 29, 44, 1, 4 );
        named_resources[ slider_empty_right ] = new image_rsrc( GUI_RESOURCE_FILE, 27, 44, 2, 4 );
        named_resources[ slider_full_left ] = new image_rsrc( GUI_RESOURCE_FILE, 30, 48, 2, 4 );
        named_resources[ slider_full_fill ] = new image_rsrc( GUI_RESOURCE_FILE, 29, 48, 1, 4 );
        named_resources[ slider_full_right ] = new image_rsrc( GUI_RESOURCE_FILE, 27, 48, 2, 4 );
        named_resources[ dial_large_dial ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 156, 42, 42 );
        named_resources[ dial_large_dot ] = new image_rsrc( GUI_RESOURCE_FILE, 42, 156, 6, 6 );
        named_resources[ dial_small_dial ] = new image_rsrc( GUI_RESOURCE_FILE, 0, 198, 22, 22 );
        named_resources[ dial_small_dot ] = new image_rsrc( GUI_RESOURCE_FILE, 42, 162, 4, 4 );
        
        submitTask( new ManageResources_task( false ) );
    }
    void deInitNamedResources()
    {
        scoped_lock< rwlock > slock( resources_lock, RW_WRITE );
        
        for( std::map< gui_resource_name, gui_resource* >::iterator iter = named_resources.begin();
            iter != named_resources.end();
            ++iter )
        {
            delete iter -> second;
        }
        
        named_resources.clear();
        
        submitTask( new ManageResources_task( true ) );
    }
    
    gui_resource* getNamedResource( gui_resource_name name )
    {
        scoped_lock< rwlock > slock( resources_lock, RW_READ );
        
        if( named_resources.count( name ) )
            return named_resources[ name ];
        else
        {
            exception e;
            ff::write( *e,
                       "getNamedResource(): No resource with name 0x",
                       ff::to_x( ( unsigned long )name ) );
            throw e;
        }
    }
    
    gui_texture* acquireTexture( std::string filename )
    {
        scoped_lock< rwlock > slock( resources_lock, RW_WRITE );
        
        gui_texture_holder* h;
        
        if( resource_textures.count( filename ) )
            h = resource_textures[ filename ];
        else
        {
            h = new gui_texture_holder();
            resource_textures[ filename ] = h;
            new_textures = true;
        }
        
        h -> ref_count++;
        
        return h -> texture;
    }
    void releaseTexture( gui_texture* t )
    {
        scoped_lock< rwlock > slock( resources_lock, RW_WRITE );
        
        for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
             iter != resource_textures.end();
             ++iter )
        {
            if( iter -> second -> texture == t )
            {
                iter -> second -> ref_count--;
                
                if( iter -> second -> ref_count < 1 )
                {
                    old_textures = true;
                    
                    if( iter -> second -> ref_count < 0 )
                        throw exception( "releaseTexture(): Texture reference count < 0" );
                }
                
                return;
            }
        }
        
        throw exception( "releaseTexture(): No such texture" );
    }
}


