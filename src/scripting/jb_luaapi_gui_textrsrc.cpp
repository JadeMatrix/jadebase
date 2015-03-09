/* 
 * jb_luaapi_gui_textrsrc.cpp
 * 
 * Implements GUI text resource API fron jb_luaapi.hpp
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
        int jade_gui_newTextRsrc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc >= 1
                    && !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_text_rsrc", "", "pointsize", 1, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 2
                    && !lua_isstring( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_text_rsrc", "", "font", 2, "string" ).c_str() );
                    return 0;
                }
                if( argc >= 3
                    && !lua_isstring( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_text_rsrc", "", "string", 3, "string" ).c_str() );
                    return 0;
                }
                
                if( argc > 3 )
                {
                    luaL_error( state, err_argcount( "new_text_rsrc", "", 2, 0, -3 ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_newuserdata( state, sizeof( shared_ptr< text_rsrc > ) );
                
                switch( argc )                                                  // This is set up to use default values for constructor
                {
                case 0:
                    new( rsrc_sp ) shared_ptr< text_rsrc >( new text_rsrc() );
                    break;
                case 1:
                    new( rsrc_sp ) shared_ptr< text_rsrc >( new text_rsrc( lua_tonumber( state, 1 ) ) );
                    break;
                case 2:
                    new( rsrc_sp ) shared_ptr< text_rsrc >( new text_rsrc( lua_tonumber( state, 1 ),
                                                                           lua_tostring( state, 2 ) ) );
                    break;
                case 3:
                    new( rsrc_sp ) shared_ptr< text_rsrc >( new text_rsrc( lua_tonumber( state, 1 ),
                                                                           lua_tostring( state, 2 ),
                                                                           lua_tostring( state, 3 ) ) );
                    break;
                default:
                    break;
                }
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_textrsrc_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_textrsrc_pointSize );
                    lua_setfield( state, -2, "point_size" );
                    lua_pushcfunction( state, jade_gui_textrsrc_string );
                    lua_setfield( state, -2, "string" );
                    lua_pushcfunction( state, jade_gui_textrsrc_font );
                    lua_setfield( state, -2, "font" );
                    lua_pushcfunction( state, jade_gui_textrsrc_color );
                    lua_setfield( state, -2, "color" );
                    lua_pushcfunction( state, jade_gui_textrsrc_maxDimensions );
                    lua_setfield( state, -2, "max_dimensions" );
                    lua_pushcfunction( state, jade_gui_textrsrc_baseline );
                    lua_setfield( state, -2, "baseline" );
                    lua_pushcfunction( state, jade_gui_textrsrc_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_textrsrc_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_TEXT_RSRC );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "text_rsrc" ).c_str() );
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
        int jade_gui_textrsrc_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "dimensions", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "dimensions", "text_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                std::pair< unsigned int, unsigned int > dims( ( *rsrc_sp ) -> getDimensions() );
                
                lua_pushnumber( state, dims.first );
                lua_pushnumber( state, dims.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_pointSize( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "point_size", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                    case 2:
                        if( !lua_isnumber( state, 2 ) )
                        {
                            luaL_error( state, err_argtype( "point_size", "text_rsrc", "size", 1, "number" ).c_str() );
                            return 0;
                        }
                        ( *rsrc_sp ) -> setPointSize( lua_tonumber( state, 2 ) );
                    case 1:
                        lua_pushnumber( state, ( *rsrc_sp ) -> getPointSize() );
                        break;
                    default:
                        luaL_error( state, err_argcount( "point_size", "text_rsrc", 2, 0, 1 ).c_str() );
                        return 0;
                }
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_string( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "string", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                    case 2:
                        if( !lua_isstring( state, 2 ) )
                        {
                            luaL_error( state, err_argtype( "string", "text_rsrc", "string", 1, "string" ).c_str() );
                            return 0;
                        }
                        ( *rsrc_sp ) -> setString( lua_tostring( state, 2 ) );
                    case 1:
                        lua_pushstring( state, ( *rsrc_sp ) -> getString().c_str() );
                        break;
                    default:
                        luaL_error( state, err_argcount( "string", "text_rsrc", 2, 0, 1 ).c_str() );
                        return 0;
                }
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_font( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "font", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                    case 2:
                        if( !lua_isstring( state, 2 ) )
                        {
                            luaL_error( state, err_argtype( "font", "text_rsrc", "font", 1, "string" ).c_str() );
                            return 0;
                        }
                        ( *rsrc_sp ) -> setFont( lua_tostring( state, 2 ) );
                    case 1:
                        lua_pushstring( state, ( *rsrc_sp ) -> getFont().c_str() );
                        break;
                    default:
                        luaL_error( state, err_argcount( "font", "text_rsrc", 2, 0, 1 ).c_str() );
                        return 0;
                }
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_color( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "color", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc != 5 )
                {
                    luaL_error( state, err_argcount( "color", "text_rsrc", 1, 4 ).c_str() );
                    return 0;
                }
                
                if( !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "color", "text_rsrc", "red", 1, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "color", "text_rsrc", "green", 2, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "color", "text_rsrc", "blue", 3, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 5 ) )
                {
                    luaL_error( state, err_argtype( "color", "text_rsrc", "alpha", 4, "number" ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                ( *rsrc_sp ) -> setColor( lua_tonumber( state, 2 ),
                                          lua_tonumber( state, 3 ),
                                          lua_tonumber( state, 4 ),
                                          lua_tonumber( state, 5 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_maxDimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "max_dimensions", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc == 3
                    || argc == 4 )
                {
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "max_dimensions", "text_rsrc", "width", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "max_dimensions", "text_rsrc", "heigth", 2, "number" ).c_str() );
                        return 0;
                    }
                    
                    if( argc == 4
                        && !lua_isnumber( state, 4 ) )
                    {
                        luaL_error( state, err_argtype( "max_dimensions", "text_rsrc", "ellipsis_mode", 3, "number" ).c_str() );
                        return 0;
                    }
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                    case 4:
                        ( *rsrc_sp ) -> setMaxDimensions( lua_tonumber( state, 2 ),
                                                          lua_tonumber( state, 3 ),
                                                          ( text_rsrc::ellipsis_mode )lua_tonumber( state, 4 ) );
                        break;
                    case 3:
                        ( *rsrc_sp ) -> setMaxDimensions( lua_tonumber( state, 2 ),
                                                          lua_tonumber( state, 3 ) );
                        break;
                    case 1:
                        break;
                    default:
                        luaL_error( state, err_argcount( "max_dimensions", "text_rsrc", 3, 0, 2, 3 ).c_str() );
                        return 0;
                }
                
                std::pair< int, int > max_d = ( *rsrc_sp ) -> getMaxDimensions();
                
                lua_pushnumber( state, max_d.first );
                lua_pushnumber( state, max_d.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_baseline( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "baseline", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                    case 2:
                        if( !lua_isboolean( state, 2 ) )
                        {
                            luaL_error( state, err_argtype( "baseline", "text_rsrc", "enabled", 1, "boolean" ).c_str() );
                            return 0;
                        }
                        ( *rsrc_sp ) -> setEnableBaseline( lua_toboolean( state, 2 ) );
                    case 1:
                        lua_pushboolean( state, ( *rsrc_sp ) -> getEnableBaseline() );
                        break;
                    default:
                        luaL_error( state, err_argcount( "baseline", "text_rsrc", 2, 0, 1 ).c_str() );
                        return 0;
                }
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "__gc", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "text_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                rsrc_sp -> ~shared_ptr< text_rsrc >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                {
                    luaL_error( state, err_objtype( "__tostring", "text_rsrc" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "text_rsrc", 0 ).c_str() );
                    return 0;
                }
                
                shared_ptr< text_rsrc >* rsrc_sp = ( shared_ptr< text_rsrc >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< unsigned int, unsigned int > dims( ( *rsrc_sp ) -> getDimensions() );
                
                ff::write( str,
                           "jade::text_rsrc at 0x",
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


