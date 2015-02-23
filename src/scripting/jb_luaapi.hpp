#ifndef JB_LUAAPI_HPP
#define JB_LUAAPI_HPP

/* 
 * jb_luaapi.hpp
 * 
 * Declarations for Lua API functions, some of which are implemented in
 * jb_luaapi.cpp and others in their specific subsystem's .CPP.
 * 
 * Note on implementation: all API functions should be implemented in
 * jb_luaapi.cpp unless necessary.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <lua.hpp>

/******************************************************************************//******************************************************************************/

// Lua API exception safety
#define LUA_API_SAFETY_BLOCK_BEGIN  try
#define LUA_API_SAFETY_BLOCK_END    catch( jade::exception e ) \
                                    { \
                                        luaL_error( state, e.what() ); \
                                        return 0; \
                                    } \
                                    catch( std::exception e ) \
                                    { \
                                        luaL_error( state, e.what() ); \
                                        return 0; \
                                    }

namespace jade
{
    namespace lua                                                               // To prevent potential pollution of namespace "jade"
    {
        // FILETYPES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // GUI /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // MAIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // SCRIPTING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // TASKING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // THREADING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_util_log( lua_State* );                                        // Takes N arguments and prints them with ff::write( jb_out, ... )
        int jade_util_log_dev( lua_State* );                                    // Like jade_util_log_dev() but only if jade::getDevMode()
        
        int jade_util_quit( lua_State* );                                       // Requests that jadebase quit
        
        int jade_util_loadSettingsFile( lua_State* );                           // Replicates jade::loadSettingsFile()
        int jade_util_reloadSettingsFiles( lua_State* );                        // Replicates jade::reloadSettingsFiles()
        int jade_util_setSetting( lua_State* );                                 // Replicates the jade::setSetting() functions
        int jade_util_getSetting( lua_State* );                                 // Replicates the jade::getSetting_*() functions
        
        // These are function rather than constants as program name/ver. can be
        // changed during runtime, and the jadebase version is styled to match.
        int jade_util_getJadebaseVersion( lua_State* );                         // Multreturn jadebase version as "jadebase", major, minor, patch
        int jade_util_getProgramVersion( lua_State* );                          // Multreturn host program version as name, major, minor, patch
        
        // WINDOWSYS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
    }
}

/******************************************************************************//******************************************************************************/

#endif


