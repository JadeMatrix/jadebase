/* 
 * jb_luaapi_gui_group.cpp
 * 
 * Implements GUI group API fron jb_luaapi.hpp
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
        void groupToUData( lua_State* state,
                            const std::shared_ptr< group >& g )                 // Pushes onto stack
        {
            new( lua_newuserdata( state, sizeof( std::shared_ptr< group > ) ) ) std::shared_ptr< group >( g );
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, jade_gui_group_position );
                lua_setfield( state, -2, "position" );
                lua_pushcfunction( state, jade_gui_group_dimensions );
                lua_setfield( state, -2, "dimensions" );
                lua_pushcfunction( state, jade_gui_group_addElement );
                lua_setfield( state, -2, "add_element" );
                lua_pushcfunction( state, jade_gui_group_removeElement );
                lua_setfield( state, -2, "remove_element" );
                lua_pushcfunction( state, jade_gui_group_drawBackground );
                lua_setfield( state, -2, "draw_background" );
                lua_pushcfunction( state, jade_gui_group_setShownCallback );
                lua_setfield( state, -2, "set_shown_callback" );
                lua_pushcfunction( state, jade_gui_group_setHiddenCallback );
                lua_setfield( state, -2, "set_hidden_callback" );
                lua_pushcfunction( state, jade_gui_group_setClosedCallback );
                lua_setfield( state, -2, "set_closed_callback" );
                lua_pushcfunction( state, jade_gui_group_gc );
                lua_setfield( state, -2, "__gc" );
                lua_pushcfunction( state, jade_gui_group_tostring );
                lua_setfield( state, -2, "__tostring" );
                
                lua_pushnumber( state, JADE_GROUP );
                lua_setfield( state, -2, "__type_key" );
                
                lua_pushstring( state, warn_metatable( __FILE__, "group" ).c_str() );
                lua_setfield( state, -2, "__metatable" );                       // Protect metatable
                
                lua_pushstring( state, "__index" );                             // Create object index
                lua_pushvalue( state, -2 );
                lua_settable( state, -3 );
            }
            lua_setmetatable( state, -2 );
        }
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int jade_gui_newGroup( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 4 )
                {
                    luaL_error( state, err_argcount( "new_group", "", 1, 4 ).c_str() );
                    return 0;
                }
                
                if( !lua_isnumber( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "new_group", "", "x", 1, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 2 ) )
                {
                    luaL_error( state, err_argtype( "new_group", "", "y", 2, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 3 ) )
                {
                    luaL_error( state, err_argtype( "new_group", "", "width", 3, "number" ).c_str() );
                    return 0;
                }
                if( !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, err_argtype( "new_group", "", "height", 4, "number" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group > group_sp = std::make_shared< group >( nullptr,
                                                                               lua_tonumber( state, 1 ),
                                                                               lua_tonumber( state, 2 ),
                                                                               lua_tonumber( state, 3 ),
                                                                               lua_tonumber( state, 4 ) );
                
                groupToUData( state, group_sp );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "position", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "position", "group", "x", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "position", "group", "y", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *group_sp ) -> setRealPosition( lua_tonumber( state, 2 ),
                                                      lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< int, int > pos( ( *group_sp ) -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "position", "group", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "dimensions", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "group", "width", 1, "number" ).c_str() );
                        return 0;
                    }
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, err_argtype( "dimensions", "group", "height", 2, "number" ).c_str() );
                        return 0;
                    }
                    ( *group_sp ) -> setRealDimensions( lua_tonumber( state, 2 ),
                                                        lua_tonumber( state, 3 ) );
                case 1:
                    {
                        std::pair< unsigned int, unsigned int > dims( ( *group_sp ) -> getRealDimensions() );
                        lua_pushnumber( state, dims.first );
                        lua_pushnumber( state, dims.second );
                    }
                    return 2;
                default:
                    luaL_error( state, err_argcount( "dimensions", "group", 2, 0, 2 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_addElement( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "add_element", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                std::shared_ptr< gui_element > element_sp;
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "add_element", "group", 1, 1 ).c_str() );
                    return 0;
                }
                
                switch( getUDataType( state, 2 ) )
                {
                case JADE_BUTTON:
                    ( *group_sp ) -> addElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< button >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_DIAL:
                    ( *group_sp ) -> addElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< dial >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_GROUP:
                    ( *group_sp ) -> addElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< group >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_SCROLLSET:
                    ( *group_sp ) -> addElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< scrollset >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_TABSET:
                    ( *group_sp ) -> addElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< tabset >* )lua_touserdata( state, 2 ) ) );
                    break;
                default:
                    luaL_error( state, err_argtype( "add_element", "group", "element", 1, "gui_element" ).c_str() );
                    return 0;
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_removeElement( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "remove_element", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                std::shared_ptr< gui_element > element_sp;
                
                if( argc != 2 )
                {
                    luaL_error( state, err_argcount( "remove_element", "group", 1, 1 ).c_str() );
                    return 0;
                }
                
                switch( getUDataType( state, 2 ) )
                {
                case JADE_BUTTON:
                    ( *group_sp ) -> removeElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< button >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_DIAL:
                    ( *group_sp ) -> removeElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< dial >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_GROUP:
                    ( *group_sp ) -> removeElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< group >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_SCROLLSET:
                    ( *group_sp ) -> removeElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< scrollset >* )lua_touserdata( state, 2 ) ) );
                    break;
                case JADE_TABSET:
                    ( *group_sp ) -> removeElement( std::dynamic_pointer_cast< gui_element >( *( std::shared_ptr< tabset >* )lua_touserdata( state, 2 ) ) );
                    break;
                default:
                    luaL_error( state, err_argtype( "remove_element", "group", "element", 1, "gui_element" ).c_str() );
                    return 0;
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_drawBackground( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "draw_background", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, err_argtype( "draw_background", "group", "draw", 1, "boolean" ).c_str() );
                        return 0;
                    }
                    ( *group_sp ) -> setDrawBackground( lua_toboolean( state, 2 ) );
                case 1:
                    lua_pushboolean( state, ( *group_sp ) -> getDrawBackground() );
                    return 1;
                default:
                    luaL_error( state, err_argcount( "draw_background", "group", 2, 0, 1 ).c_str() );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setShownCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "set_shown_callback", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_shown_callback", "group", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 1 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_shown_callback", "group", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                ( *group_sp ) -> setShownCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setHiddenCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "set_hidden_callback", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_hidden_callback", "group", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 1 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_hidden_callback", "group", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                ( *group_sp ) -> setHiddenCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setClosedCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "set_closed_callback", "group" ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< lua_callback > cb_sp;
                
                if( argc > 1 )
                {
                    if( argc > 2 )
                    {
                        luaL_error( state, err_argcount( "set_closed_callback", "group", 2, 0, 1 ).c_str() );
                        return 0;
                    }
                    
                    if( getUDataType( state, 1 ) != JADE_CALLBACK )
                    {
                        luaL_error( state, err_argtype( "set_closed_callback", "group", "callback", 1, "callback" ).c_str() );
                        return 0;
                    }
                    
                    cb_sp = *( std::shared_ptr< lua_callback >* )lua_touserdata( state, 2 );
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                ( *group_sp ) -> setClosedCallback( std::dynamic_pointer_cast< callback >( cb_sp ) );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "__gc", "group" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "group", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* dial_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                dial_sp -> ~shared_ptr< group >();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_GROUP )
                {
                    luaL_error( state, err_objtype( "__tostring", "group" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "group", 0 ).c_str() );
                    return 0;
                }
                
                std::shared_ptr< group >* group_sp = ( std::shared_ptr< group >* )lua_touserdata( state, 1 );
                
                std::string str;
                std::pair< int, int > pos( ( *group_sp ) -> getRealPosition() );
                std::pair< unsigned int, unsigned int > dims( ( *group_sp ) -> getRealDimensions() );
                
                ff::write( str,
                           "jade::group at 0x",
                           ff::to_x( ( long )( &**group_sp ) ),
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


