/* 
 * jb_luaapi_utility.cpp
 * 
 * Implements utility API fron jb_luaapi.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_luaapi.hpp"

#include "../jadebase.hpp"                                                      // Include everything

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
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
                    ff::write( log_string,
                               luaL_tolstring( state, i + 1, NULL ) );          // Use luaL_tolstring() to get __tostring() conversions
                
                ff::write( jb_out, log_string );
            }
            else
                lua_pop( state, argc );                                         // Ignore & pop all arguments
            
            return 0;
        }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        LUA_API_SAFETY_BLOCK_END
    }
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    namespace lua
    {
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
                    luaL_error( state, err_argcount( "quit", "", 0 ).c_str() );
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
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "load_settings_file", "", "override", 2, "boolean" ).c_str() );
                        return 0;
                    }
                case 1:
                    if( !lua_isstring( state, 1 ) )
                    {
                        luaL_error( state, err_argtype( "load_settings_file", "", "file", 1, "string" ).c_str() );
                        return 0;
                    }
                    break;
                default:
                    luaL_error( state, err_argcount( "load_settings_file", "", 2, 1, -2 ).c_str() );
                    return 0;
                }
                
                switch( argc )                                                  // Pass arguments
                {
                case 1:
                    loadSettingsFile( luaL_tolstring( state, 1, NULL ),
                                      false );
                    break;
                case 2:
                    loadSettingsFile( luaL_tolstring( state, 1, NULL ),
                                      false,
                                      lua_toboolean( state, 2 ) );
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
                            luaL_error( state, err_argtype( "reload_settings_files", "", "flush", 1, "boolean" ).c_str() );
                    }
                    else
                        luaL_error( state, err_argcount( "reload_settings_files", "", 2, 0, -1 ).c_str() );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_setSetting( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc == 2 )
                {
                    if( !lua_isstring( state, 1 ) )                             // Check 'key' (1) argument
                    {
                        luaL_error( state, err_argtype( "set_setting", "", "key", 1, "string" ).c_str() );
                        return 0;
                    }
                    
                    switch( lua_type( state, 2 ) )                              // Check 'value' (2) argument & execute
                    {
                    case LUA_TNUMBER:
                        setSetting( luaL_tolstring( state, 1, NULL ), lua_tonumber( state, 2 ), true );
                        break;
                    case LUA_TBOOLEAN:
                        setSetting( luaL_tolstring( state, 1, NULL ), ( bool )lua_toboolean( state, 2 ), true );
                        break;
                    case LUA_TSTRING:
                        setSetting( luaL_tolstring( state, 1, NULL ), std::string( luaL_tolstring( state, 2, NULL ) ), true );
                        break;
                    default:
                        luaL_error( state, err_argtype( "set_setting", "", "value", 2, "number, string or boolean" ).c_str() );
                        return 0;
                    }
                    
                    saveSettings( getUserSettingsFileName() );                  // Save every time a script changes a setting
                }
                else
                    luaL_error( state, err_argcount( "set_setting", "", 1, 2 ).c_str() );
                
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
                {
                    luaL_error( state, err_argcount( "get_jadebase_version", "", 0 ).c_str() );
                    return 0;
                }
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
                {
                    luaL_error( state, err_argcount( "get_program_version", "", 0 ).c_str() );
                    return 0;
                }
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
        
        int jade_util_newCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, err_argcount( "new_callback", "", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( lua_isfunction( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_callback", "", "function", 1, "function" ).c_str() );
                    return 0;
                }
                
                lua_getglobal( state, "__jade_lua_state" );
                if( !check_udata_type( state, -1, JADE_LUA_STATE ) )
                {
                    luaL_error( state, "new_callback(): __jade_lua_state corrupt" );
                    return 0;
                }
                
                lua_state* state_p = *( lua_state** )lua_touserdata( state, -1 );
                lua_reference ref = luaL_ref( state, 1 );
                
                new( lua_newuserdata( state, sizeof( std::shared_ptr< lua_callback > ) ) ) std::shared_ptr< lua_callback >( new lua_callback( *state_p, ref ) );
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_util_callback_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_util_callback_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_CALLBACK );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "callback" ).c_str() );
                    lua_setfield( state, -2, "__metatable" );                   // Protect metatable
                    
                    lua_pushstring( state, "__index" );                         // Create object index
                    lua_pushvalue( state, -2 );
                    lua_settable( state, -3 );
                }
                lua_setmetatable( state, -2 );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_callback_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_CALLBACK ) )
                {
                    luaL_error( state, err_objtype( "__gc", "callback" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "callback", 0 ).c_str() );
                    return 0;
                }
                
                ( ( std::shared_ptr< lua_callback >* )lua_touserdata( state, 1 ) ) -> ~shared_ptr< lua_callback >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_callback_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_CALLBACK ) )
                {
                    luaL_error( state, err_objtype( "__tostring", "callback" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "callback", 0 ).c_str() );
                    return 0;
                }
                
                std::string str;
                
                ff::write( str,
                           "jade::lua_callback at 0x",
                           ff::to_x( ( long )( &*( std::shared_ptr< lua_callback >* )lua_touserdata( state, 1 ) ) ) );
                
                lua_pushstring( state, str.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


