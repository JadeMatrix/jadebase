/* 
 * jb_lua.cpp
 * 
 * Implements Lua scripting system from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_lua.hpp"

#include "../utility/jb_exception.hpp"
#include "../utility/jb_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex global_state_mutex;
    jade::lua_state* global_state = NULL;
    
    luaL_Reg lualibs[] = { { "base", luaopen_base } };
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    // LUA_STATE ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    lua_state::lua_state()
    {
        state = luaL_newstate();
        
        for( int i = 0; i < sizeof( lualibs ) / sizeof( luaL_Reg ); ++i )
        {
            lualibs[ i ].func( state );
            lua_settop( state, 0 );
        }
        
        initAPI();
    }
    lua_state::~lua_state()
    {
        lua_close( state );
    }
        
    void lua_state::open( std::string file )
    {
        scoped_lock< mutex > slock( state_mutex );
        
        int status = luaL_loadfile( state, file.c_str() );
        
        if( status )
        {
            exception e;
            ff::write( *e,
                       "Lua error in lua_state::open(): ",
                       lua_tostring( state, -1 ) );
            lua_pop( state, 1 );
            throw e;
        }
    }
    void lua_state::run()
    {
        scoped_lock< mutex > slock( state_mutex );
        
        int status = lua_pcall( state, 0, LUA_MULTRET, 0 );
        
        if( status )
        {
            exception e;
            ff::write( *e,
                       "Lua error in lua_state::run(): ",
                       lua_tostring( state, -1 ) );
            lua_pop( state, 1 );
            throw e;
        }
    }
    
    // void lua_state::call( lua_reference ref )
    // {
    //     scoped_lock< mutex > slock( state_mutex );
        
    //     lua_rawgeti( state, LUA_REGISTRYINDEX, ref );
        
    //     int status = lua_pcall( state, 0, 0, 0 );
        
    //     if( status )
    //     {
    //         exception e;
    //         ff::write( *e,
    //                    "Lua error in lua_state::call(): ",
    //                    lua_tostring( state, -1 ) );
    //         lua_pop( state, 1 );
    //         throw e;
    //     }
    // }
    
    // GLOBAL STATE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void initGlobalLuaState()
    {
        scoped_lock< mutex > slock( global_state_mutex );
        
        if( global_state != NULL )
            throw exception( "initGlobalLuaState(): Global Lua state already exists" );
        
        global_state = new lua_state();
    }
    void deinitGlobalLuaState()
    {
        scoped_lock< mutex > slock( global_state_mutex );
        
        if( global_state != NULL )
            delete global_state;
    }
    lua_state& getGlobalLuaState()
    {
        scoped_lock< mutex > slock( global_state_mutex );
        
        if( global_state == NULL )
            throw exception( "getGlobalLuaState(): No global Lua state" );
        
        return *global_state;
    }
}


