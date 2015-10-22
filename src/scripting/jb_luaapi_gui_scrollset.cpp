/* 
 * jb_luaapi_gui_scrollset.cpp
 * 
 * Implements GUI scrollset API fron jb_luaapi.hpp
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
        int jade_gui_newScrollset( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 3 && argc != 5 )
                {
                    luaL_error( state, err_argcount( "new_scrollset", "", 2, 3, 5 ).c_str() );
                    return 0;
                }
                
                if( !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_scrollset", "", "x", 1, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_scrollset", "", "y", 2, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollable > scrollable_sp;
                
                switch( getUDataType( state, argc ) )                           // Contetns will be the last argument
                {
                case JADE_GROUP:
                    scrollable_sp = std::dynamic_pointer_cast< scrollable >( *( std::shared_ptr< group >* )lua_touserdata( state, argc ) );
                    break;
                default:
                    luaL_error( state, err_argtype( "new_scrollset", "", "contents", argc, "scrollable gui_element" ).c_str() );
                    return 0;
                }
                
                switch( argc )
                {
                case 3:
                    new( lua_newuserdata( state, sizeof( std::shared_ptr< scrollset > ) ) ) std::shared_ptr< scrollset >( new scrollset( lua_tonumber( state, 1 ),
                                                                                                                                         lua_tonumber( state, 2 ),
                                                                                                                                         scrollable_sp ) );
                    break;
                case 5:
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "new_scrollset", "", "width", 3, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 4 ) )
                    {
                        luaL_error( state, err_argtype( "new_scrollset", "", "height", 4, "number" ).c_str() );
                        return 0;
                    }
                    new( lua_newuserdata( state, sizeof( std::shared_ptr< scrollset > ) ) ) std::shared_ptr< scrollset >( new scrollset( lua_tonumber( state, 1 ),
                                                                                                                                         lua_tonumber( state, 2 ),
                                                                                                                                         lua_tonumber( state, 3 ),
                                                                                                                                         lua_tonumber( state, 4 ),
                                                                                                                                         scrollable_sp ) );
                    break;
                default:
                    break;
                }
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_scrollset_position );
                    lua_setfield( state, -2, "position" );
                    lua_pushcfunction( state, jade_gui_scrollset_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_scrollset_barsAlwaysVisible );
                    lua_setfield( state, -2, "bars_always_visible" );
                    lua_pushcfunction( state, jade_gui_scrollset_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_scrollset_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_SCROLLSET );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "scrollset" ).c_str() );
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
        int jade_gui_scrollset_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "position", "scrollset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollset >* sset_sp = ( std::shared_ptr< scrollset >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "position", "scrollset", "x", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "position", "scrollset", "y", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *sset_sp ) -> setRealPosition( lua_tonumber( state, 2 ),
                                                     lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< int, int > pos( ( *sset_sp ) -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "position", "scrollset", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_scrollset_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "dimensions", "scrollset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollset >* sset_sp = ( std::shared_ptr< scrollset >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "scrollset", "width", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "scrollset", "height", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *sset_sp ) -> setRealDimensions( lua_tonumber( state, 2 ),
                                                       lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< unsigned int, unsigned int > dims( ( *sset_sp ) -> getRealDimensions() );
                        lua_pushnumber( state, dims.first );
                        lua_pushnumber( state, dims.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "dimensions", "scrollset", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_scrollset_barsAlwaysVisible( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "bars_always_visible", "scrollset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollset >* sset_sp = ( std::shared_ptr< scrollset >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "bars_always_visible", "scrollset", "draw", 1, "boolean" ).c_str() );
                        return 0;
                    }
                    ( *sset_sp ) -> setBarsAlwaysVisible( lua_toboolean( state, 2 ) );
                case 1:
                    lua_pushboolean( state, ( *sset_sp ) -> getBarsAlwaysVisible() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "bars_always_visible", "scrollset", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_scrollset_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "__gc", "scrollset" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "scrollset", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollset >* sset_sp = ( std::shared_ptr< scrollset >* )lua_touserdata( state, 1 );
                
                sset_sp -> ~shared_ptr< scrollset >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_scrollset_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "__tostring", "scrollset" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "scrollset", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< scrollset >* sset_sp = ( std::shared_ptr< scrollset >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< int, int > pos( ( *sset_sp ) -> getRealPosition() );
                std::pair< unsigned int, unsigned int > dims( ( *sset_sp ) -> getRealDimensions() );
                
                ff::write( str,
                           "jade::scrollset at 0x",
                           ff::to_x( ( long )( &**sset_sp ),
                                     PTR_HEX_WIDTH,
                                     PTR_HEX_WIDTH ),
                           " (",
                           pos.first,
                           ",",
                           pos.second,
                           ";",
                           dims.first,
                           ",",
                           dims.second,
                           ")" );
                
                lua_pushstring( state, str.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


