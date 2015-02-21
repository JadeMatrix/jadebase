/* 
 * jb_luaapi.cpp
 * 
 * Implements initLuaAPI() from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../jadebase.hpp"                                                      // Include everything

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    int test_print( lua_State* state )
    {
        int argc = lua_gettop( state );
        
        if( argc != 1 )
            luaL_error( state, "Arguments != 1" );
        else
            ff::write( jb_out, lua_tostring( state, 1 ), "\n" );
        
        return 0;
    }
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    void lua_state::initAPI()
    {
        lua_register( state, "test_print", test_print );
        
        // ...
    }
}


