/* 
 * jb_lua_initapi.cpp
 * 
 * Implements jade::lua_state::initAPI()
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_lua.hpp"

#include "../jadebase.hpp"                                                      // Include everything

/******************************************************************************//******************************************************************************/

namespace jade
{
    void lua_state::initAPI()
    {
        // "jade" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        lua_newtable( state );
        {
            // "filetypes" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                // "png" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                lua_newtable( state );
                {
                    // Functions
                    
                    lua_pushcfunction( state, lua::jade_filetypes_png_new );
                    lua_setfield( state, -2, "new" );
                    
                    // Constants
                    
                    lua_pushnumber( state, png_file::GRAY );
                    lua_setfield( state, -2, "GRAY" );
                    lua_pushnumber( state, png_file::PALETTE );
                    lua_setfield( state, -2, "PALETTE" );
                    lua_pushnumber( state, png_file::RGB );
                    lua_setfield( state, -2, "RGB" );
                    lua_pushnumber( state, png_file::RGB_ALPHA );
                    lua_setfield( state, -2, "RGB_ALPHA" );
                    lua_pushnumber( state, png_file::GRAY_ALPHA );
                    lua_setfield( state, -2, "GRAY_ALPHA" );
                }
                lua_setfield( state, -2, "png" );
            }
            lua_setfield( state, -2, "filetypes" );
            
            // "gui" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                // Constructors
                
                lua_pushcfunction( state, lua::jade_gui_newTextRsrc );
                lua_setfield( state, -2, "new_text_rsrc" );
                lua_pushcfunction( state, lua::jade_gui_newImageRsrc );
                lua_setfield( state, -2, "new_image_rsrc" );
                lua_pushcfunction( state, lua::jade_gui_newGroup );
                lua_setfield( state, -2, "new_group" );
                lua_pushcfunction( state, lua::jade_gui_newButton );
                lua_setfield( state, -2, "new_button" );
                
                // Constants
                
                lua_pushnumber( state, TOP_LEFT );
                lua_setfield( state, -2, "TOP_LEFT" );
                lua_pushnumber( state, TOP_CENTER );
                lua_setfield( state, -2, "TOP_CENTER" );
                lua_pushnumber( state, TOP_RIGHT );
                lua_setfield( state, -2, "TOP_RIGHT" );
                lua_pushnumber( state, CENTER_LEFT );
                lua_setfield( state, -2, "CENTER_LEFT" );
                lua_pushnumber( state, CENTER_CENTER );
                lua_setfield( state, -2, "CENTER_CENTER" );
                lua_pushnumber( state, CENTER_RIGHT );
                lua_setfield( state, -2, "CENTER_RIGHT" );
                lua_pushnumber( state, BOTTOM_LEFT );
                lua_setfield( state, -2, "BOTTOM_LEFT" );
                lua_pushnumber( state, BOTTOM_CENTER );
                lua_setfield( state, -2, "BOTTOM_CENTER" );
                lua_pushnumber( state, BOTTOM_RIGHT );
                lua_setfield( state, -2, "BOTTOM_RIGHT" );
                
                lua_pushstring( state, TEXTBOX_FONT );
                lua_setfield( state, -2, "TEXTBOX_FONT" );
                lua_pushstring( state, GUI_LABEL_FONT );
                lua_setfield( state, -2, "GUI_LABEL_FONT" );
                lua_pushstring( state, GUI_DEBUG_FONT );
                lua_setfield( state, -2, "GUI_DEBUG_FONT" );
                
                lua_pushnumber( state, text_rsrc::NONE );
                lua_setfield( state, -2, "ELLIPSIS_NONE" );
                lua_pushnumber( state, text_rsrc::BEGINNING );
                lua_setfield( state, -2, "ELLIPSIS_BEGINNING" );
                lua_pushnumber( state, text_rsrc::MIDDLE );
                lua_setfield( state, -2, "ELLIPSIS_MIDDLE" );
                lua_pushnumber( state, text_rsrc::END );
                lua_setfield( state, -2, "ELLIPSIS_END" );
            }
            lua_setfield( state, -2, "gui" );
            
            // "main" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "scripting" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "tasking" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "tasking" );
            
            // "threading" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "utility" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, lua::jade_util_log );
                lua_setfield( state, -2, "log" );
                lua_pushcfunction( state, lua::jade_util_log_dev );
                lua_setfield( state, -2, "log_dev" );
                
                lua_pushcfunction( state, lua::jade_util_quit );
                lua_setfield( state, -2, "quit" );
                
                lua_pushcfunction( state, lua::jade_util_loadSettingsFile );
                lua_setfield( state, -2, "load_settings_file" );
                lua_pushcfunction( state, lua::jade_util_reloadSettingsFiles );
                lua_setfield( state, -2, "reload_settings_files" );
                lua_pushcfunction( state, lua::jade_util_setSetting );
                lua_setfield( state, -2, "set_setting" );
                lua_pushcfunction( state, lua::jade_util_getSetting );
                lua_setfield( state, -2, "get_setting" );
                
                lua_pushcfunction( state, lua::jade_util_getJadebaseVersion );
                lua_setfield( state, -2, "get_jadebase_version" );
                lua_pushcfunction( state, lua::jade_util_getProgramVersion );
                lua_setfield( state, -2, "get_program_version" );
            }
            lua_setfield( state, -2, "utility" );
            
            // "windowsys" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, lua::jade_windowsys_newWindow );
                lua_setfield( state, -2, "new_window" );
            }
            lua_setfield( state, -2, "windowsys" );
        }
        lua_setglobal( state, "jade" );
    }
}


