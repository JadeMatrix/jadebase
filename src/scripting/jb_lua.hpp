#ifndef JADEBASE_LUA_HPP
#define JADEBASE_LUA_HPP

/* 
 * jb_lua.hpp
 * 
 * Lua scripting subsystem
 * 
 */

// TODO: Consider moving jade::lua_state::open() and jade::lua_state::run() to the constructor

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include <lua.hpp>

#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    typedef int lua_reference;
    
    class lua_state
    {
        friend class CallLuaFunction_task;
        friend class lua_callback;
    public:
        lua_state();
        ~lua_state();
        
        void open( std::string file );                                          // Open a Lua script and feed it into the lua state
        void run();                                                             // Call the state as-is
        
        // void call( lua_reference );                                             // Call a Lua function with no arguments and no return by reference (e.g. a
        //                                                                         // callback)
    protected:
        lua_State* state;
        jade::mutex state_mutex;
        
        void initAPI();                                                         // Implemented in jb_lua_initapi.cpp
    };
    
    void      initGlobalLuaState();
    void    deinitGlobalLuaState();
    lua_state& getGlobalLuaState();
}

/******************************************************************************//******************************************************************************/

#endif


