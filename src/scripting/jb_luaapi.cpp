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
    // USERDATA TYPE ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct userdata_t
    {
        enum
        {
            
        } type;
        
        union
        {
            
        };
    };
    
    // API FUNCTION DEFINITIONS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    int jade_util_log( lua_State* state )
    {
        int argc = lua_gettop( state );
        
        std::string log_string = "Lua log message: ";
        
        for( int i = 0; i < argc; ++i )
            ff::write( log_string, lua_tostring( state, i + 1 ) );
        
        ff::write( jb_out, log_string );
        
        return 0;
    }
    int jade_util_quit( lua_State* state )
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
        // "jade" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        lua_newtable( state );
        {
            // "filetypes" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "filetypes" );
            
            // "gui" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "gui" );
            
            // "main" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "main" );
            
            // "scripting" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "scripting" );
            
            // "tasking" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "tasking" );
            
            // "threading" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // lua_newtable( state );
            // {
            // }
            // lua_setfield( state, -2, "threading" );
            
            // "utility" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, jade_util_log );
                lua_setfield( state, -2, "log" );
                
                lua_pushcfunction( state, jade_util_quit );
                lua_setfield( state, -2, "quit" );
            }
            lua_setfield( state, -2, "utility" );
            
            // "windowsys" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "windowsys" );
        }
        lua_setglobal( state, "jade" );
    }
}


