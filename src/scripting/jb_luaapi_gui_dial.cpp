/* 
 * jb_luaapi_gui_dial.cpp
 * 
 * Implements GUI dial API fron jb_luaapi.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_luaapi.hpp"

#include "../jadebase.hpp"                                                      // Include everything

/******************************************************************************//******************************************************************************/

namespace jade
{
    namespace lua
    {
        int jade_gui_newDial( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc >= 1
                    && !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_dial", "", "x", 1, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 2
                    && !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_dial", "", "y", 2, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 3
                    && !lua_isboolean( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_dial", "", "small", 3, "boolean" ).c_str() );
                    return 0;
                }
                if( argc >= 4
                    && !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "new_dial", "", "start_value", 4, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_newuserdata( state, sizeof( std::shared_ptr< dial > ) );
                
                switch( argc )                                                  // This is set up to use default values for constructor
                {
                case 2:
                    new( dial_sp ) std::shared_ptr< dial >( new dial( NULL,
                                                                      lua_tonumber( state, 1 ),
                                                                      lua_tonumber( state, 2 ) ) );
                    break;
                case 3:
                    new( dial_sp ) std::shared_ptr< dial >( new dial( NULL,
                                                                      lua_tonumber( state, 1 ),
                                                                      lua_tonumber( state, 2 ),
                                                                      lua_toboolean( state, 3 ) ) );
                    break;
                case 4:
                    new( dial_sp ) std::shared_ptr< dial >( new dial( NULL,
                                                                      lua_tonumber( state, 1 ),
                                                                      lua_tonumber( state, 2 ),
                                                                      lua_toboolean( state, 3 ),
                                                                      lua_tonumber( state, 4 ) ) );
                    break;
                default:
                    luaL_error( state, err_argcount( "new_dial", "", 2, 2, -4 ).c_str() );
                    return 0;
                }
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_dial_position );
                    lua_setfield( state, -2, "position" );
                    lua_pushcfunction( state, jade_gui_dial_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_dial_value );
                    lua_setfield( state, -2, "value" );
                    lua_pushcfunction( state, jade_gui_dial_small );
                    lua_setfield( state, -2, "small" );
                    lua_pushcfunction( state, jade_gui_dial_setValueChangeCallback );
                    lua_setfield( state, -2, "set_value_change_callback" );
                    lua_pushcfunction( state, jade_gui_dial_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_dial_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_DIAL );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "dial" ).c_str() );
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
        int jade_gui_dial_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "position", "dial" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "position", "dial", "x", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "position", "dial", "y", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *dial_sp ) -> setRealPosition( lua_tonumber( state, 2 ),
                                                     lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< int, int > pos( ( *dial_sp ) -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "position", "dial", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "dimensions", "button" ).c_str() );
                    return 0;
                }
                
                if( argc != 1 )
                {
                    luaL_error( state, err_argcount( "dimensions", "button", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                std::pair< unsigned int, unsigned int > dims( ( *butt_sp ) -> getRealDimensions() );
                lua_pushnumber( state, dims.first );
                lua_pushnumber( state, dims.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_value( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "value", "dial" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "value", "dial", "value", 1, "number" ).c_str() );
                        return 0;
                    }
                    ( *dial_sp ) -> setValue( lua_tonumber( state, 2 ) );
                case 1:
                    lua_pushnumber( state, ( *dial_sp ) -> getValue() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "value", "dial", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_small( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "small", "dial" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "small", "dial", "small", 1, "boolean" ).c_str() );
                        return 0;
                    }
                    ( *dial_sp ) -> setSmall( lua_toboolean( state, 2 ) );
                case 1:
                    lua_pushboolean( state, ( *dial_sp ) -> getSmall() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "small", "dial", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_setValueChangeCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "set_value_change_callback", "dial" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_value_change_callback", "dial", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 2 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_value_change_callback", "dial", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                ( *dial_sp ) -> setValueChangeCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "__gc", "dial" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "dial", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                dial_sp -> ~shared_ptr< dial >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_dial_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_DIAL )
                {
                    luaL_error( state, err_objtype( "__tostring", "dial" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "dial", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< dial >* dial_sp = ( std::shared_ptr< dial >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< int, int > pos( ( *dial_sp ) -> getRealPosition() );
                bool small = ( *dial_sp ) -> getSmall();
                
                ff::write( str,
                           "jade::dial at 0x",
                           ff::to_x( ( long )( &**dial_sp ) ),
                           " (",
                           pos.first,
                           ",",
                           pos.second,
                           "; ",
                           small ? "small" : "large",
                           ")" );
                
                lua_pushstring( state, str.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


