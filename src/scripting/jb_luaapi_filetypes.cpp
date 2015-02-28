/* 
 * jb_luaapi_filetypes.cpp
 * 
 * Implements filetypes API fron jb_luaapi.hpp
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
        int jade_filetypes_png_new( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "png.new() requires exactly 1 argument" );
                    return 0;
                }
                if( !lua_isstring( state, 1 ) )
                {
                    luaL_error( state, "'filename' not a string for png.new()" );
                    return 0;
                }
                
                png_file** file_p = ( png_file** )lua_newuserdata( state, sizeof( png_file* ) );
                
                ( *file_p ) = new png_file( lua_tostring( state, 1 ) );
                
                lua_newtable( state );                                          // Create metatable
                {
                    lua_pushcfunction( state, jade_filetypes_png_getDimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_filetypes_png_getBitDepth );
                    lua_setfield( state, -2, "bit_depth" );
                    lua_pushcfunction( state, jade_filetypes_png_getColorType );
                    lua_setfield( state, -2, "color_type" );
                    lua_pushcfunction( state, jade_filetypes_png_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_filetypes_png_toString );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_PNG_FILE );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change png_file's metatable" );
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
        int jade_filetypes_png_getDimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )                                   // First argument in png_file, so no others
                {
                    luaL_error( state, "png:get_dimensions() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_dimensions() on a non-png_file type" );
                    return 0;
                }
                
                std::pair< unsigned int, unsigned int > dimensions = ( *( png_file** )lua_touserdata( state, 1 ) ) -> getDimensions();
                lua_pushnumber( state, dimensions.first );
                lua_pushnumber( state, dimensions.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_getBitDepth( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:get_bit_depth() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_bit_depth() on a non-png_file type" );
                    return 0;
                }
                
                lua_pushnumber( state, ( *( png_file** )lua_touserdata( state, 1 ) ) -> getBitDepth() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_getColorType( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:get_color_type() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_color_type() on a non-png_file type" );
                    return 0;
                }
                
                lua_pushnumber( state, ( *( png_file** )lua_touserdata( state, 1 ) ) -> getColorType() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__gc() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:__gc() on a non-png_file type" );
                    return 0;
                }
                
                delete *( png_file** )lua_touserdata( state, 1 );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_toString( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__tostring() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:__tostring() on a non-png_file type" );
                    return 0;
                }
                
                std::string png_string;
                
                ff::write( png_string,
                           "jade::png_file at 0x",
                           ff::to_x( ( long )( *( png_file** )lua_touserdata( state, 1 ) ) ) );
                
                lua_pushstring( state, png_string.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


