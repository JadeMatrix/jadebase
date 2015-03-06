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
        void group_to_udata( lua_State* state, shared_ptr< group > g )          // Pushes onto stack
        {
            new( lua_newuserdata( state, sizeof( shared_ptr< group > ) ) ) shared_ptr< group >( g );
            
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
                lua_setfield( state, -2, "shown_callback" );
                lua_pushcfunction( state, jade_gui_group_setHiddenCallback );
                lua_setfield( state, -2, "hidden_callback" );
                lua_pushcfunction( state, jade_gui_group_setClosedCallback );
                lua_setfield( state, -2, "closed_callback" );
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
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_position( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_dimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_addElement( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_removeElement( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_drawBackground( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setShownCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setHiddenCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setClosedCallback( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_tostring( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


