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
        bool use_sub_names = getSetting_bln( "jb_LuaAPISubNames" );
        
        // "jade" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        lua_newtable( state );
        {
            lua_state** state_ptr = ( lua_state** )lua_newuserdata( state, sizeof( lua_state* ) );
            ( *state_ptr ) = this;
            lua_setfield( state, -2, "__jade_lua_state" );
            
            lua_newtable( state );
            {
                lua_pushnumber( state, lua::JADE_LUA_STATE );
                lua_setfield( state, -2, "__type_key" );
                
                lua_pushstring( state, lua::warn_metatable( __FILE__, "__jade_lua_state" ).c_str() );
                lua_setfield( state, -2, "__metatable" );                       // Protect metatable
                
                lua_pushstring( state, "__index" );                             // Create object index
                lua_pushvalue( state, -2 );
                lua_settable( state, -3 );
            }
            lua_setmetatable( state, -2 );
            
            // "filetypes" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            if( use_sub_names ) lua_newtable( state );
            {
                // "png" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                lua_pushcfunction( state, lua::jade_filetypes_newPNG );
                lua_setfield( state, -2, "new_png" );
                
                lua_newtable( state );                                          // Constants
                {
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
            if( use_sub_names ) lua_setfield( state, -2, "filetypes" );
            
            // "gui" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            if( use_sub_names ) lua_newtable( state );
            {
                // Constructors
                
                lua_pushcfunction( state, lua::jade_gui_newTextRsrc );
                lua_setfield( state, -2, "new_text_rsrc" );
                lua_pushcfunction( state, lua::jade_gui_newImageRsrc );
                lua_setfield( state, -2, "new_image_rsrc" );
                lua_pushcfunction( state, lua::jade_gui_newButton );
                lua_setfield( state, -2, "new_button" );
                lua_pushcfunction( state, lua::jade_gui_newDial );
                lua_setfield( state, -2, "new_dial" );
                lua_pushcfunction( state, lua::jade_gui_newGroup );
                lua_setfield( state, -2, "new_group" );
                lua_pushcfunction( state, lua::jade_gui_newScrollset );
                lua_setfield( state, -2, "new_scrollset" );
                lua_pushcfunction( state, lua::jade_gui_newTabset );
                lua_setfield( state, -2, "new_tabset" );
                
                lua_pushcfunction( state, lua::jade_gui_newTab );
                lua_setfield( state, -2, "new_tab" );
                
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
            if( use_sub_names ) lua_setfield( state, -2, "gui" );
            
            // "main" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "scripting" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "tasking" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            if( use_sub_names ) lua_newtable( state );
            {
                
            }
            if( use_sub_names ) lua_setfield( state, -2, "tasking" );
            
            // "threading" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "utility" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            if( use_sub_names ) lua_newtable( state );
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
            if( use_sub_names ) lua_setfield( state, -2, "utility" );
            
            // "windowsys" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            if( use_sub_names ) lua_newtable( state );
            {
                lua_pushcfunction( state, lua::jade_windowsys_newWindow );
                lua_setfield( state, -2, "new_window" );
            }
            if( use_sub_names ) lua_setfield( state, -2, "windowsys" );
        }
        lua_setglobal( state, "jade" );
    }
}


