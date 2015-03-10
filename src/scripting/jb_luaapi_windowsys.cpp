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
                    luaL_error( state, err_argcount( "new_window", "", 0 ).c_str() );
                    return 0;
                }
                
                // Create a new userdata space then placement new a window
                // container from a new window
                container< window >* cont_p = new( lua_newuserdata( state, sizeof( container< window > ) ) ) container< window >( new window() );
                
                window* window_p = cont_p -> acquire();
                if( window_p )                                                  // The window COULD possibly be NULL, but unlikely
                    submitTask( new window::manipulate( window_p ) );           // Submit a manipulate
                cont_p -> release();
                
                lua_newtable( state );                                          // Create metatable
                {
                    lua_pushcfunction( state, jade_windowsys_window_getTopGroup );
                    lua_setfield( state, -2, "top_group" );
                    lua_pushcfunction( state, jade_windowsys_window_setTitle );
                    lua_setfield( state, -2, "set_title" );
                    lua_pushcfunction( state, jade_windowsys_window_close );
                    lua_setfield( state, -2, "close" );
                    lua_pushcfunction( state, jade_windowsys_window_requestRedraw );
                    lua_setfield( state, -2, "request_redraw" );
                    lua_pushcfunction( state, jade_windowsys_window_isOpen );
                    lua_setfield( state, -2, "is_open" );
                    lua_pushcfunction( state, jade_windowsys_window_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_windowsys_window_toString );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_WINDOW );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, warn_metatable( __FILE__, "window" ).c_str() );
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
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "top_group", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "top_group", "window", 0 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                if( *slock )
                {
                    group_to_udata( state, ( *slock ) -> getTopGroup() );
                    return 1;
                }
                else
                {
                    if( getSetting_bln( "jb_LuaClosedWindowSilentFail" ) )
                    {
                        luaL_error( state, "Call of window:top_group() on a closed window" );
                        return 0;
                    }
                    else
                    {
                        lua_pushnil( state );
                        return 1;
                    }
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_setTitle( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "set_title", "window" ).c_str() );
                    return 0;
                }
                
                if( argc != 2 )                                                 // Object + string, so 2 = 1
                {
                    luaL_error( state, err_argcount( "set_title", "window", 1, 1 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                if( *slock )
                {
                    window::manipulate* wm = new window::manipulate( *slock );
                    wm -> setTitle( luaL_tolstring( state, 2, NULL ) );
                    submitTask( wm );
                }
                else
                {
                    if( getSetting_bln( "jb_LuaClosedWindowSilentFail" ) )
                        luaL_error( state, "Call of window:set_title() on a closed window" );
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_close( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "close", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "close", "window", 0 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                if( *slock )
                {
                    window::manipulate* wm = new window::manipulate( *slock );
                    wm -> close();
                    submitTask( wm );
                }
                else
                {
                    if( getSetting_bln( "jb_LuaClosedWindowSilentFail" ) )
                        luaL_error( state, "Call of window:close() on a closed window" );
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_requestRedraw( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "request_redraw", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "request_redraw", "window", 0 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                if( *slock )
                    ( *slock ) -> requestRedraw();
                else
                {
                    if( getSetting_bln( "jb_LuaClosedWindowSilentFail" ) )
                        luaL_error( state, "Call of window:request_redraw() on a closed window" );
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_isOpen( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "is_open", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "is_open", "window", 0 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                lua_pushboolean( state, !!*slock );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "__gc", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__gc", "window", 0 ).c_str() );
                    return 0;
                }
                
                container< window >* cont_p = ( container< window >* )lua_touserdata( state, 1 );
                
                cont_p -> ~container< window >();                               // Explicit desctuctor call; Lua frees memory
                
                // We do NOT close the window, as the window type in Lua is
                // merely a handle to the real window; window:__gc being called
                // only means the Lua handle has been fully dereferenced.
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_toString( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || getUDataType( state, 1 ) != JADE_WINDOW )
                {
                    luaL_error( state, err_objtype( "__tostring", "window" ).c_str() );
                    return 0;
                }
                
                if( argc > 1 )
                {
                    luaL_error( state, err_argcount( "__tostring", "window", 1, 1 ).c_str() );
                    return 0;
                }
                
                scoped_lock< container< window > > slock( *( container< window >* )lua_touserdata( state, 1 ) );
                
                std::string window_string;
                
                if( *slock )                                                    // Test whether window is still available
                {
                    ff::write( window_string,
                               "handle to jade::window '",
                               ( *slock ) -> getTitle(),
                               "'" );                                           // Don't try to get the platform window for an ID for now, as if new_window()
                                                                                // and window:__tostring() on that window are called in a script executed on the
                                                                                // main thread, the platform window probably will not have been created yet.
                }
                else
                {
                    ff::write( window_string,
                               "handle to a closed jade::window" );
                }
                
                lua_pushstring( state, window_string.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


