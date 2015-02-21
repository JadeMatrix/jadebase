#ifndef JB_LUA_HPP
#define JB_LUA_HPP

/* 
 * jb_lua.hpp
 * 
 * Lua scripting subsystem
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

#include <lua.hpp>

#include "../threading/jb_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class lua_state
    {
    public:
        lua_state();
        ~lua_state();
        
        void open( std::string file );
        
        void run();
    protected:
        void initAPI();                                                         // Implemented in jb_luaapi.cpp
    private:
        lua_State* state;
        jade::mutex state_mutex;
    };
    
    void      initGlobalLuaState();
    void    deinitGlobalLuaState();
    lua_state& getGlobalLuaState();
}

/******************************************************************************//******************************************************************************/

#endif


