/* 
 * jb_luaapi_windowsys.cpp
 * 
 * Implements window system API fron jb_luaapi.hpp
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
        int jade_windowsys_newWindow( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 0 )
                {
                    luaL_error( state, "new_window() requires exactly 0 arguments" );
                    return 0;
                }
                
                window** file_p = ( window** )lua_newuserdata( state, sizeof( window* ) );
                
                ( *file_p ) = new window();                                     // Create window
                submitTask( new window::manipulate( *file_p ) );                // Submit a manipulate
                
                lua_newtable( state );                                          // Create metatable
                {
                    lua_pushcfunction( state, jade_windowsys_window_getTopGroup );
                    lua_setfield( state, -2, "top_group" );
                    lua_pushcfunction( state, jade_windowsys_window_setTitle );
                    lua_setfield( state, -2, "set_title" );
                    lua_pushcfunction( state, jade_windowsys_window_requestRedraw );
                    lua_setfield( state, -2, "request_redraw" );
                    lua_pushcfunction( state, jade_windowsys_window_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_windowsys_window_toString );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_WINDOW );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change window's metatable" );
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
        int jade_windowsys_window_getTopGroup( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "window:top_group() takes exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:top_group() on a non-window type" );
                    return 0;
                }
                
                group* g = ( *( window** )lua_touserdata( state, 1 ) ) -> getTopGroup();
                
                ff::write( jb_out, "got window top group 0x", ff::to_x( ( long )g ), "\n" );
                
                group_to_udata( state, g );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_setTitle( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 2 )                                  // Object + string, so 2 = 1
                {
                    luaL_error( state, "window:set_title() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:set_title() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:set_title(): Userdata window is NULL" );
                    return 0;
                }
                
                window::manipulate* wm = new window::manipulate( *w );
                
                wm -> setTitle( luaL_tolstring( state, 2, NULL ) );
                
                submitTask( wm );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_requestRedraw( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "window:request_redraw() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:request_redraw() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:request_redraw(): Userdata window is NULL" );
                    return 0;
                }
                
                ( *w ) -> requestRedraw();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__gc() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:__gc() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:__gc(): Userdata window is NULL" );
                    return 0;
                }
                
                window::manipulate* wm = new window::manipulate( *w );
                
                wm -> close();
                
                submitTask( wm );
                
                ( *w ) = NULL;                                                  // NULL-out userdata
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_toString( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "window:__tostring() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:__tostring() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:__tostring(): Userdata window is NULL" );
                    return 0;
                }
                
                std::string window_string;
                
                ff::write( window_string,
                           "jade::window '",
                           ( *w ) -> getTitle(),
                           "'" );                                               // Don't try to get the platform window for an ID for now, as if new_window()
                                                                                // and window:__tostring() on that window are called in a script executed on the
                                                                                // main thread, the platform window probably will not have been created yet.
                
                lua_pushstring( state, window_string.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


