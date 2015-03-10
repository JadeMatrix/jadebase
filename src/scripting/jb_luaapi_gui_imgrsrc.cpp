/* 
 * jb_luaapi_gui_imgrsrc.cpp
 * 
 * Implements GUI image resource API fron jb_luaapi.hpp
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
        int jade_gui_newImageRsrc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 5 )
                {
                    luaL_error( state, err_argcount( "new_image_rsrc", "", 1, 5 ).c_str() );
                    return 0;
                }
                
                if( !lua_isstring( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_image_rsrc", "", "file", 1, "string" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_image_rsrc", "", "x", 2, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_image_rsrc", "", "y", 3, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "new_image_rsrc", "", "width", 4, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 5 ) )
                {
                    luaL_error( state, err_argtype( "new_image_rsrc", "", "heigth", 5, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< image_rsrc >* rsrc_sp = ( std::shared_ptr< image_rsrc >* )lua_newuserdata( state, sizeof( std::shared_ptr< image_rsrc > ) );
                
                new( rsrc_sp ) std::shared_ptr< image_rsrc >( new image_rsrc( lua_tostring( state, 1 ),
                                                                              lua_tonumber( state, 2 ),
                                                                              lua_tonumber( state, 3 ),
                                                                              lua_tonumber( state, 4 ),
                                                                              lua_tonumber( state, 5 ) ) );
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_imgrsrc_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_imgrsrc_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_imgrsrc_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_IMAGE_RSRC );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "image_rsrc" ).c_str() );
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
        int jade_gui_imgrsrc_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_IMAGE_RSRC )
                {
                    luaL_error( state, err_objtype( "dimensions", "image_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "dimensions", "image_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< image_rsrc >* rsrc_sp = ( std::shared_ptr< image_rsrc >* )lua_touserdata( state, 1 );
                
                std::pair< unsigned int, unsigned int > dims( ( *rsrc_sp ) -> getDimensions() );
                
                lua_pushnumber( state, dims.first );
                lua_pushnumber( state, dims.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_imgrsrc_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_IMAGE_RSRC )
                {
                    luaL_error( state, err_objtype( "__gc", "image_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "image_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< image_rsrc >* rsrc_sp = ( std::shared_ptr< image_rsrc >* )lua_touserdata( state, 1 );
                
                rsrc_sp -> ~shared_ptr< image_rsrc >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_imgrsrc_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_IMAGE_RSRC )
                {
                    luaL_error( state, err_objtype( "__tostring", "image_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "image_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< image_rsrc >* rsrc_sp = ( std::shared_ptr< image_rsrc >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< unsigned int, unsigned int > dims( ( *rsrc_sp ) -> getDimensions() );
                
                ff::write( str,
                           "jade::image_rsrc at 0x",
                           ff::to_x( ( long )( &**rsrc_sp ) ),
                           " (",
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


