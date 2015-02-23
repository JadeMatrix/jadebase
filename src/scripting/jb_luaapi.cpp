/* 
 * jb_luaapi.cpp
 * 
 * Implements initLuaAPI() from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../jadebase.hpp"                                                      // Include everything

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

/* Template (c&p) for safety blocks
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
*/

namespace
{
    struct userdata_t
    {
        enum
        {
            
        } type;
        
        union
        {
            
        };
    };
    
    int jade_util_log_base( lua_State* state, bool dev_log )
    {
        LUA_API_SAFETY_BLOCK_BEGIN
        {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            int argc = lua_gettop( state );
            
            if( !dev_log
                || ( dev_log && jade::getDevMode() ) )                          // Print message
            {
                std::string log_string = "Lua log message: ";
                
                for( int i = 0; i < argc; ++i )
                    ff::write( log_string, lua_tostring( state, i + 1 ) );
                
                ff::write( jb_out, log_string );
            }
            else
                lua_pop( state, argc );                                         // Ignore & pop all arguments
            
            return 0;
        }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        LUA_API_SAFETY_BLOCK_END
    }
}

/* jb_luaapi.hpp **************************************************************//******************************************************************************/

namespace jade
{
    namespace lua
    {
        // FILETYPES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // GUI /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // MAIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // SCRIPTING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // TASKING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // THREADING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_util_log( lua_State* state )
        {
            return jade_util_log_base( state, false );
        }
        int jade_util_log_dev( lua_State* state )
        {
            return jade_util_log_base( state, true );
        }
        
        int jade_util_quit( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "quit() requires exactly 0 arguments" );
                else
                {
                    if( getDevMode() )
                        ff::write( jb_out, "Quit requested from Lua script\n" );
                    jb_setQuitFlag();
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_util_loadSettingsFile( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                switch( argc )                                                  // Check arguments
                {
                case 3:
                    if( !lua_isboolean( state, 3 ) )
                    {
                        luaL_error( state, "'override' (3) not a boolean for load_settings_file()" );
                        return 0;
                    }
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, "'save' (2) not a boolean for load_settings_file()" );
                        return 0;
                    }
                case 1:
                    if( !lua_isstring( state, 1 ) )
                    {
                        luaL_error( state, "'file' (1) not a string for load_settings_file()" );
                        return 0;
                    }
                    break;
                default:
                    luaL_error( state, "load_settings_file() requires 1-3 arguments" );
                    return 0;
                }
                
                switch( argc )                                                  // Pass arguments
                {
                case 1:
                    loadSettingsFile( lua_tostring( state, 1 ) );
                    break;
                case 2:
                    loadSettingsFile( lua_tostring( state, 1 ),
                                      lua_toboolean( state, 2 ) );
                    break;
                case 3:
                    loadSettingsFile( lua_tostring( state, 1 ),
                                      lua_toboolean( state, 2 ),
                                      lua_toboolean( state, 3 ) );
                    break;
                default:
                    // Won't get here because we check range above
                    break;
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_reloadSettingsFiles( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc == 0 )
                    reloadSettingsFiles();
                else
                    if( argc == 1 )
                    {
                        if( lua_isboolean( state, 1 ) )
                            reloadSettingsFiles( lua_toboolean( state, 1 ) );
                        else
                            luaL_error( state, "'flush' (1) not a boolean for reload_settings_files()" );
                    }
                    else
                        luaL_error( state, "reload_settings_files() requires 0-1 arguments" );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_setSetting( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc >= 2
                    && argc <= 3 )
                {
                    if( !lua_isstring( state, 1 ) )                             // Check 'key' (1) argument
                    {
                        luaL_error( state, "'key' (1) not a string for set_setting()" );
                        return 0;
                    }
                    
                    bool save;
                    if( argc == 2 )
                        save = SETSETTING_DEFAULT_SAVEVAL;
                    else
                    {
                        if( lua_isboolean( state, 3 ) )                         // Check 'save' (3) argument
                            save = lua_toboolean( state, 3 );
                        else
                        {
                            luaL_error( state, "'save' (3) not a boolean for set_setting()" );
                            return 0;
                        }
                    }
                    
                    switch( lua_type( state, 2 ) )                              // Check 'value' 2) argument & execute
                    {
                    case LUA_TNUMBER:
                        setSetting( lua_tostring( state, 1 ), lua_tonumber( state, 2 ), save );
                        break;
                    case LUA_TBOOLEAN:
                        setSetting( lua_tostring( state, 1 ), ( bool )lua_toboolean( state, 2 ), save );
                        break;
                    case LUA_TSTRING:
                        setSetting( lua_tostring( state, 1 ), lua_tostring( state, 2 ), save );
                        break;
                    default:
                        luaL_error( state, "'value' (2) not a number, string, or boolean for set_setting()" );
                        return 0;
                    }
                    
                    saveSettings( getUserSettingsFileName() );                  // Save every time a script changes a setting
                }
                else
                    luaL_error( state, "set_setting() requires 2-3 arguments" );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        // int jade_util_getSetting( lua_State* )                               // Implemented in jb_settings.cpp
        
        int jade_util_getJadebaseVersion( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "get_jadebase_version() requires exactly 0 arguments" );
                else
                {
                    lua_pushstring( state, "jadebase" );
                    
                    version& jadebase_version( getJadebaseVersion() );
                    
                    lua_pushnumber( state, jadebase_version.major );
                    lua_pushnumber( state, jadebase_version.minor );
                    lua_pushnumber( state, jadebase_version.patch );
                }
                
                return 4;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_getProgramVersion( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "get_program_version() requires exactly 0 arguments" );
                else
                {
                    lua_pushstring( state, getProgramName().c_str() );
                    
                    version program_version( getProgramVersion() );
                    
                    lua_pushnumber( state, program_version.major );
                    lua_pushnumber( state, program_version.minor );
                    lua_pushnumber( state, program_version.patch );
                }
                
                return 4;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        // WINDOWSYS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
    }
}

/* jb_lua.hpp *****************************************************************//******************************************************************************/

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
                
            }
            lua_setfield( state, -2, "filetypes" );
            
            // "gui" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "gui" );
            
            // "main" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "main" );
            
            // "scripting" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "scripting" );
            
            // "tasking" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "tasking" );
            
            // "threading" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "threading" );
            
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
                
            }
            lua_setfield( state, -2, "windowsys" );
        }
        lua_setglobal( state, "jade" );
    }
}


