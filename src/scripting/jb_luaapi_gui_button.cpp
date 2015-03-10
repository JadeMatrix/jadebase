/* 
 * jb_luaapi_gui_buutton.cpp
 * 
 * Implements GUI button API fron jb_luaapi.hpp
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
        int jade_gui_newButton( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc >= 1
                    && !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_button", "", "x", 1, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 2
                    && !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_button", "", "y", 2, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 3
                    && !lua_isnumber( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_button", "", "width", 3, "number" ).c_str() );
                    return 0;
                }
                if( argc >= 4
                    && !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "new_button", "", "height", 4, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_newuserdata( state, sizeof( std::shared_ptr< button > ) );
                
                switch( argc )                                                  // This is set up to use default values for constructor
                {
                case 2:
                    new( butt_sp ) std::shared_ptr< button >( new button( NULL,
                                                                          lua_tonumber( state, 1 ),
                                                                          lua_tonumber( state, 2 ) ) );
                    break;
                case 3:
                    new( butt_sp ) std::shared_ptr< button >( new button( NULL,
                                                                          lua_tonumber( state, 1 ),
                                                                          lua_tonumber( state, 2 ),
                                                                          lua_tonumber( state, 3 ) ) );
                    break;
                case 4:
                    new( butt_sp ) std::shared_ptr< button >( new button( NULL,
                                                                          lua_tonumber( state, 1 ),
                                                                          lua_tonumber( state, 2 ),
                                                                          lua_tonumber( state, 3 ),
                                                                          lua_tonumber( state, 4 ) ) );
                    break;
                default:
                    luaL_error( state, err_argcount( "new_button", "", 2, 2, -4 ).c_str() );
                    return 0;
                }
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_button_position );
                    lua_setfield( state, -2, "position" );
                    lua_pushcfunction( state, jade_gui_button_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_button_setContents );
                    lua_setfield( state, -2, "set_contents" );
                    lua_pushcfunction( state, jade_gui_button_setToggleOnCallback );
                    lua_setfield( state, -2, "set_toggle_on_callback" );
                    lua_pushcfunction( state, jade_gui_button_setToggleOffCallback );
                    lua_setfield( state, -2, "set_toggle_off_callback" );
                    lua_pushcfunction( state, jade_gui_button_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_button_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_BUTTON );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "button" ).c_str() );
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
        int jade_gui_button_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "position", "button" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "position", "button", "x", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "position", "button", "y", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *butt_sp ) -> setRealPosition( lua_tonumber( state, 2 ),
                                                     lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< int, int > pos( ( *butt_sp ) -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "position", "button", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "dimensions", "button" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "button", "width", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "button", "height", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *butt_sp ) -> setRealDimensions( lua_tonumber( state, 2 ),
                                                       lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< unsigned int, unsigned int > dims( ( *butt_sp ) -> getRealDimensions() );
                        lua_pushnumber( state, dims.first );
                        lua_pushnumber( state, dims.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "dimensions", "button", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_setContents( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "set_contents", "button" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                std::shared_ptr< gui_resource > element_sp;
                
                if( argc == 3 || argc == 4 )
                {
                    switch( getUDataType( state, 2 ) )
                    {
                    case JADE_IMAGE_RSRC:
                        {
                            std::shared_ptr< image_rsrc >* rsrc_sp = ( std::shared_ptr< image_rsrc >* )lua_touserdata( state, 2 );
                            element_sp = std::dynamic_pointer_cast< gui_resource >( *rsrc_sp );
                        }
                        break;
                    case JADE_TEXT_RSRC:
                        {
                            std::shared_ptr< text_rsrc >* rsrc_sp = ( std::shared_ptr< text_rsrc >* )lua_touserdata( state, 2 );
                            element_sp = std::dynamic_pointer_cast< gui_resource >( *rsrc_sp );
                        }
                        break;
                    default:
                        luaL_error( state, err_argtype( "set_contents", "button", "contents", 1, "image_rsrc or text_rsrc" ).c_str() );
                        return 0;
                    }
                    
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "set_contents", "button", "align", 1, "number" ).c_str() );
                        return 0;
                    }
                    
                    if( argc == 4
                        && !lua_isboolean( state, 4 ) )
                    {
                        luaL_error( state, err_argtype( "set_contents", "button", "resize", 1, "boolean" ).c_str() );
                        return 0;
                    }
                }
                
                switch( argc )
                {
                case 4:
                    ( *butt_sp ) -> setContents( element_sp,
                                                 ( resource_align )lua_tonumber( state, 3 ),
                                                 lua_toboolean( state, 4 ) );
                    return 0;
                case 3:
                    ( *butt_sp ) -> setContents( element_sp,
                                                 ( resource_align )lua_tonumber( state, 3 ) );
                    return 0;
                case 1:
                    ( *butt_sp ) -> setContents( element_sp,
                                                 CENTER_CENTER );
                    return 0;
                default:
                    luaL_error( state, err_argcount( "set_contents", "button", 2, 0, 2 ).c_str() );
                    return 0;
                    
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_setToggleOnCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "set_toggle_on_callback", "button" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_toggle_on_callback", "button", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 1 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_toggle_on_callback", "button", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                ( *butt_sp ) -> setToggleOnCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_setToggleOffCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "set_toggle_off_callback", "button" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_toggle_off_callback", "button", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 1 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_toggle_off_callback", "button", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                ( *butt_sp ) -> setToggleOffCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "__gc", "button" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "button", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                butt_sp -> ~shared_ptr< button >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_BUTTON )
                {
                    luaL_error( state, err_objtype( "__tostring", "button" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "button", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< button >* butt_sp = ( std::shared_ptr< button >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< int, int > pos( ( *butt_sp ) -> getRealPosition() );
                std::pair< unsigned int, unsigned int > dims( ( *butt_sp ) -> getRealDimensions() );
                
                ff::write( str,
                           "jade::button at 0x",
                           ff::to_x( ( long )( &**butt_sp ) ),
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


