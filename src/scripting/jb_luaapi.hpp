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

#include "jb_lua.hpp"
#include "../gui/jb_element.hpp"

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
    class lua_callback : public gui_callback
    {
    public:
        lua_callback( lua_state&,                                               // Constructor takes a jade::lua_state& instead of lua_State* for thread safety
                      lua_reference );                                          // Reference to the Lua object to call (function with no arguments & no return)
        ~lua_callback();
        
        void call();
    protected:
        lua_state& parent_state;
        lua_reference lua_func;
    };
    
    namespace lua                                                               // To prevent potential pollution of namespace "jade"
    {
        // FILETYPES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_filetypes_png_new( lua_State* );                               // Opens a PNG file from a filename and creates a userdata with the methods:
        int jade_filetypes_png_getDimensions( lua_State* );                     //      Multreturns width, height
        int jade_filetypes_png_getBitDepth( lua_State* );                       //      Returns a number
        int jade_filetypes_png_getColorType( lua_State* );                      //      Returns one of the following API constants:
                                                                                //          jade.filetypes.png.GRAY
                                                                                //          jade.filetypes.png.PALETTE
                                                                                //          jade.filetypes.png.RGB
                                                                                //          jade.filetypes.png.RGB_ALPHA
                                                                                //          jade.filetypes.png.GRAY_ALPHA
        int jade_filetypes_png_gc( lua_State* );                                // Garbage collection function for jade.filetypes.png
        int jade_filetypes_png_toString( lua_State* );                          // ToString function for jade.filetypes.png; returns "jade::png_file at 0x****"
        // TODO: Functions to convert to & from type of libpng Lua bindings
        
        // GUI /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // MAIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // No API functions
        
        // SCRIPTING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // No API functions for now, potentially inter-script if support for
        // more scripting lanugages is added
        
        // TASKING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // TODO: Figure out a way given a lua_State* to get its associated jade::lua_state
        
        // class CallLuaFunction_task : public task
        // {
        // public:
        //     CallLuaFunction_task( lua_State* );                                 // Expects the lua_State to be passed directly from jade_tasking_submitTask()
        //     bool execute( task_mask* );
        //     task_mask getMask()
        //     {
        //         return TASK_ANY;
        //     }
        //     // Medium priority
        // protected:
        //     lua_State
        // };
        
        // int jade_tasking_submitTask( lua_State* );                              // Takes a Lua function and any number of arguments, creating & submitting a
        //                                                                         // CallLuaFunction_task from them
        
        // THREADING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // No API functions, as jadebase Lua scripts are event-based and should
        // use the tasking facilities of the framework rather than Lua's
        // threading.
        
        // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_util_log( lua_State* );                                        // Takes N arguments and prints them with ff::write( jb_out, ... )
        int jade_util_log_dev( lua_State* );                                    // Like jade_util_log_dev() but only if jade::getDevMode()
        
        int jade_util_quit( lua_State* );                                       // Requests that jadebase quit
        
        int jade_util_loadSettingsFile( lua_State* );                           // Replicates jade::loadSettingsFile() with save = false
        int jade_util_reloadSettingsFiles( lua_State* );                        // Replicates jade::reloadSettingsFiles()
        int jade_util_setSetting( lua_State* );                                 // Replicates the jade::setSetting() functions with save = true
        int jade_util_getSetting( lua_State* );                                 // Replicates the jade::getSetting_*() functions
        
        // These are function rather than constants as program name/ver. can be
        // changed during runtime, and the jadebase version is styled to match.
        int jade_util_getJadebaseVersion( lua_State* );                         // Multreturn jadebase version as "jadebase", major, minor, patch
        int jade_util_getProgramVersion( lua_State* );                          // Multreturn host program version as name, major, minor, patch
        
        // WINDOWSYS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_windowsys_newWindow( lua_State* );
        int jade_windowsys_window_getTopGroup( lua_State* );
        // int jade_windowsys_window_setFullscreen( lua_State* );
        int jade_windowsys_window_setTitle( lua_State* );
        int jade_windowsys_window_gc( lua_State* );
        int jade_windowsys_window_toString( lua_State* );
    }
}

/******************************************************************************//******************************************************************************/

#endif


