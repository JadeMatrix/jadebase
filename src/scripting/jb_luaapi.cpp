/* 
 * jb_luaapi.cpp
 * 
 * Implements initLuaAPI() from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../jadebase.hpp"                                                      // Include everything

/******************************************************************************//******************************************************************************/

namespace jade
{
    lua_callback::lua_callback( lua_state& s, lua_reference r ) : parent_state( s )
    {
        lua_func = r;
    }
    lua_callback::~lua_callback()
    {
        luaL_unref( parent_state.state, LUA_REGISTRYINDEX, lua_func );
    }
    
    void lua_callback::call()
    {
        scoped_lock< mutex > slock( parent_state.state_mutex );
        
        lua_rawgeti( parent_state.state, LUA_REGISTRYINDEX, lua_func );
        
        int status = lua_pcall( parent_state.state, 0, 0, 0 );
        
        if( status )
        {
            exception e;
            ff::write( *e,
                       "Lua error in lua_callback::call(): ",
                       lua_tostring( parent_state.state, -1 ) );
            lua_pop( parent_state.state, 1 );
            throw e;
        }
    }
    
    bool check_udata_type( lua_State* state, int index, luaapi_type type )
    {
        bool result = false;
        
        if( lua_getmetatable( state, index ) )
        {
            lua_getfield( state, -1, "__type_key" );
            
            if( lua_isnumber( state, -1 ) )
                result = ( int )lua_tonumber( state, -1 ) == ( int )type;
            else
                result = false;
            
            lua_pop( state, 2 );
        }
        
        return result;
    }
}


