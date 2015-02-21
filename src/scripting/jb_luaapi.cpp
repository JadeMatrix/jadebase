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
    int jb_util_log( lua_State* state )
    {
        int argc = lua_gettop( state );
        
        std::string log_string = "Lua log message: ";
        
        for( int i = 0; i < argc; ++i )
            ff::write( log_string, lua_tostring( state, i + 1 ) );
        
        ff::write( jb_out, log_string );
        
        return 0;
    }
    int jb_util_quit( lua_State* state )
    {
        int argc = lua_gettop( state );
        
        if( argc != 0 )
            luaL_error( state, "jb_util_quit(): Function takes no arguments" );
        else
        {
            if( jade::getDevMode() )
                ff::write( jb_out, "Quit requested from Lua script\n" );
            jb_setQuitFlag();
        }
        
        return 0;
    }
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    void lua_state::initAPI()
    {
        lua_register( state, "jb_util_log", jb_util_log );
        lua_register( state, "jb_util_quit", jb_util_quit );
        
        // ...
    }
}


