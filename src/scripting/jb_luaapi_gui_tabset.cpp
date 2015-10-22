/* 
 * jb_luaapi_gui_tabset.cpp
 * 
 * Implements GUI tabset & tab API fron jb_luaapi.hpp
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
        int jade_gui_newTabset( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 4 )
                {
                    luaL_error( state, err_argcount( "new_tabset", "", 1, 4 ).c_str() );
                    return 0;
                }
                
                if( !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_tabset", "", "x", 1, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_tabset", "", "y", 2, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_tabset", "", "width", 3, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "new_tabset", "", "height", 4, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_newuserdata( state, sizeof( std::shared_ptr< tabset > ) );
                new( tset_sp ) std::shared_ptr< tabset >( new tabset( lua_tonumber( state, 1 ),
                                                                      lua_tonumber( state, 2 ),
                                                                      lua_tonumber( state, 3 ),
                                                                      lua_tonumber( state, 4 ) ) );
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_tabset_position );
                    lua_setfield( state, -2, "position" );
                    lua_pushcfunction( state, jade_gui_tabset_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_gui_tabset_addTab );
                    lua_setfield( state, -2, "add_tab" );
                    lua_pushcfunction( state, jade_gui_tabset_removeTab );
                    lua_setfield( state, -2, "remove_tab" );
                    lua_pushcfunction( state, jade_gui_tabset_makeTabCurrent );
                    lua_setfield( state, -2, "make_tab_current" );
                    lua_pushcfunction( state, jade_gui_tabset_moveTabLeft );
                    lua_setfield( state, -2, "move_tab_left" );
                    lua_pushcfunction( state, jade_gui_tabset_moveTabRight );
                    lua_setfield( state, -2, "move_tab_right" );
                    lua_pushcfunction( state, jade_gui_tabset_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_tabset_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_TABSET );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "tabset" ).c_str() );
                    lua_setfield( state, -2, "__metatable" );                   // Protect metatable
                    
                    lua_pushstring( state, "__index" );                         // Create object index
                    lua_pushvalue( state, -2 );
                    lua_settable( state, -3 );
                }
                lua_setmetatable( state, -2 );
                
                // DEBUG:
                ff::write( jb_out,
                           ">>> Lua jade.tabset at 0x",
                           ff::to_X( ( unsigned long )( &**tset_sp ),
                                     PTR_HEX_WIDTH,
                                     PTR_HEX_WIDTH ),
                           " created\n" );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "position", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "position", "tabset", "x", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "position", "tabset", "y", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *tset_sp ) -> setRealPosition( lua_tonumber( state, 2 ),
                                                     lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< int, int > pos( ( *tset_sp ) -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "position", "tabset", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_SCROLLSET )
                {
                    luaL_error( state, err_objtype( "dimensions", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "tabset", "width", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "tabset", "height", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *tset_sp ) -> setRealDimensions( lua_tonumber( state, 2 ),
                                                       lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< unsigned int, unsigned int > dims( ( *tset_sp ) -> getRealDimensions() );
                        lua_pushnumber( state, dims.first );
                        lua_pushnumber( state, dims.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "dimensions", "tabset", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_addTab( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "add_tab", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "add_tab", "tabset", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( getUDataType( state, 2 ) != JADE_TAB )
                {
                    luaL_error( state, err_argtype( "add_tab", "tabset", "tab", 1, "tab" ).c_str() );
                    return 0;
                }
                
                ( *tset_sp ) -> addTab( *( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 2 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_removeTab( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "remove_tab", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "remove_tab", "tabset", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( getUDataType( state, 2 ) != JADE_TAB )
                {
                    luaL_error( state, err_argtype( "remove_tab", "tabset", "tab", 1, "tab" ).c_str() );
                    return 0;
                }
                
                ( *tset_sp ) -> removeTab( *( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 2 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_makeTabCurrent( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "make_tab_current", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "make_tab_current", "tabset", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( getUDataType( state, 2 ) != JADE_TAB )
                {
                    luaL_error( state, err_argtype( "make_tab_current", "tabset", "tab", 1, "tab" ).c_str() );
                    return 0;
                }
                
                ( *tset_sp ) -> makeTabCurrent( *( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 2 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_moveTabLeft( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "move_tab_left", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "move_tab_left", "tabset", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( getUDataType( state, 2 ) != JADE_TAB )
                {
                    luaL_error( state, err_argtype( "move_tab_left", "tabset", "tab", 1, "tab" ).c_str() );
                    return 0;
                }
                
                ( *tset_sp ) -> moveTabLeft( *( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 2 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_moveTabRight( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "move_tab_right", "tabset" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "move_tab_right", "tabset", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( getUDataType( state, 2 ) != JADE_TAB )
                {
                    luaL_error( state, err_argtype( "move_tab_right", "tabset", "tab", 1, "tab" ).c_str() );
                    return 0;
                }
                
                ( *tset_sp ) -> moveTabRight( *( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 2 ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "__gc", "tabset" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "tabset", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                // DEBUG:
                ff::write( jb_out,
                           ">>> Lua jade.tabset at 0x",
                           ff::to_X( ( unsigned long )( &**tset_sp ),
                                     PTR_HEX_WIDTH,
                                     PTR_HEX_WIDTH ),
                           " collected\n" );
                
                tset_sp -> ~shared_ptr< tabset >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tabset_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TABSET )
                {
                    luaL_error( state, err_objtype( "__tostring", "tabset" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "tabset", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset >* tset_sp = ( std::shared_ptr< tabset >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< int, int > pos( ( *tset_sp ) -> getRealPosition() );
                std::pair< unsigned int, unsigned int > dims( ( *tset_sp ) -> getRealDimensions() );
                
                ff::write( str,
                           "jade::tabset at 0x",
                           ff::to_x( ( long )( &**tset_sp ),
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

        int jade_gui_newTab( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "new_tab", "", 1, 4 ).c_str() );
                    return 0;
                }
                
                if( !lua_isstring( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_tab", "", "title", 1, "string" ).c_str() );
                    return 0;
                }
                if( getUDataType( state, 2 ) != JADE_GROUP )
                {
                    luaL_error( state, err_argtype( "new_tab", "", "contents", 2, "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset::tab >* tab_sp = ( std::shared_ptr< tabset::tab >* )lua_newuserdata( state, sizeof( std::shared_ptr< tabset::tab > ) );
                new( tab_sp ) std::shared_ptr< tabset::tab >( new tabset::tab( NULL,
                                                                               lua_tostring( state, 1 ),
                                                                               *( std::shared_ptr< group >* )lua_touserdata( state, 2 ) ) );
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_tab_title );
                    lua_setfield( state, -2, "title" );
                    lua_pushcfunction( state, jade_gui_tab_safe );
                    lua_setfield( state, -2, "safe" );
                    lua_pushcfunction( state, jade_gui_tab_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_tab_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_TAB );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "tab" ).c_str() );
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
        int jade_gui_tab_title( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TAB )
                {
                    luaL_error( state, err_objtype( "title", "tab" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset::tab >* tab_sp = ( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isstring( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "title", "tab", "title", 1, "string" ).c_str() );
                        return 0;
                    }
                    ( *tab_sp ) -> setTitle( lua_tostring( state, 2 ) );
                case 1:
                    lua_pushstring( state, ( *tab_sp ) -> getTitle().c_str() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "title", "tab", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tab_safe( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TAB )
                {
                    luaL_error( state, err_objtype( "safe", "tab" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset::tab >* tab_sp = ( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "safe", "tab", "safe", 1, "boolean" ).c_str() );
                        return 0;
                    }
                    ( *tab_sp ) -> setSafe( lua_toboolean( state, 2 ) );
                case 1:
                    lua_pushboolean( state, ( *tab_sp ) -> getSafe() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "safe", "tab", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tab_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TAB )
                {
                    luaL_error( state, err_objtype( "__gc", "tab" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "tab", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset::tab >* tab_sp = ( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 1 );
                
                tab_sp -> ~shared_ptr< tabset::tab >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_tab_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_TAB )
                {
                    luaL_error( state, err_objtype( "__tostring", "tab" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "tab", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< tabset::tab >* tab_sp = ( std::shared_ptr< tabset::tab >* )lua_touserdata( state, 1 );
                
                std::string str;
                
                ff::write( str,
                           "jade::tab at 0x",
                           ff::to_x( ( unsigned long )( &**tab_sp ),
                                     PTR_HEX_WIDTH,
                                     PTR_HEX_WIDTH ),
                           " '",
                           ( *tab_sp ) -> getTitle(),
                           "' (",
                           ( *tab_sp ) -> getSafe() ? "safe" : "unsafe",
                           ")" );
                
                lua_pushstring( state, str.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


