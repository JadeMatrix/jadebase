#ifndef JADEBASE_LUAAPI_HPP
#define JADEBASE_LUAAPI_HPP

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

#include <cstdarg>

#include "jb_lua.hpp"
#include "../gui/jb_element.hpp"
#include "../gui/jb_group.hpp"
#include "../utility/jb_sharedpointer.hpp"

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

/* Template for safety blocks (copy & paste)
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
*/

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
        enum udata_type
        {
            JADE_PNG_FILE,
            JADE_WINDOW,
            JADE_TEXT_RSRC,
            JADE_IMAGE_RSRC,
            JADE_BUTTON,
            JADE_DIAL,
            JADE_GROUP,
            JADE_SCROLLSET,
            JADE_TABSET
        };
        
        bool check_udata_type( lua_State*, int, udata_type );                   // Utility function for checking userdata
        
        // Utility functions for standardizing error messages
        std::string err_argcount( std::string,                                  // Lua function name
                                  std::string,                                  // Object type (can be empty)
                                  int,                                          // Number of following arguments
                                  ... );                                        // x = "exactly x"; x,y = "x or y"; x,-y = "x-y"; x,y,z = "x, y or z"; etc.
        std::string err_argtype( std::string,                                   // Lua function name
                                 std::string,                                   // Argument name
                                 std::string,                                   // Object type (can be empty)
                                 int,                                           // Argument index
                                 std::string );                                 // Expected argument type
        std::string err_objtype( std::string,                                   // Lua function name
                                 std::string );                                 // Expected object type
        std::string warn_metatable( std::string,                                // File where metatable is constructed, e.g. __FILE__, __FUNCTION__, or __func__
                                    std::string );                              // Object type
        
        void group_to_udata( lua_State*, shared_ptr< group > );                 // Utility function for converting GUI groups into userdata (implemented in
                                                                                // jb_luaapi_gui.cpp)
        
        // FILETYPES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_filetypes_png_new( lua_State* );                               // Opens a PNG file from a filename and creates a userdata with the methods:
        int jade_filetypes_png_getDimensions( lua_State* );                     //     Multreturns width, height
        int jade_filetypes_png_getBitDepth( lua_State* );                       //     Returns a number
        int jade_filetypes_png_getColorType( lua_State* );                      //     Returns one of the following API constants:
                                                                                //         jade.filetypes.png.GRAY
                                                                                //         jade.filetypes.png.PALETTE
                                                                                //         jade.filetypes.png.RGB
                                                                                //         jade.filetypes.png.RGB_ALPHA
                                                                                //         jade.filetypes.png.GRAY_ALPHA
        int jade_filetypes_png_gc( lua_State* );                                // Garbage collection function for jade.filetypes.png
        int jade_filetypes_png_toString( lua_State* );                          // ToString function for jade.filetypes.png; returns "jade::png_file at 0x****"
        
        // GUI /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_gui_element_position( lua_State* );                            // Getter/setter for element position (takes x,y returns x,y)
        int jade_gui_element_dimensions( lua_State* );                          // Getter/setter for element dimensions (takes w,h returns w,h)
        int jade_gui_element_gc( lua_State* );
        
        int jade_gui_resource_dimensions( lua_State* );                         // Getter for resource dimensions (takes w,h)
        int jade_gui_resource_gc( lua_State* );
        
        int jade_gui_newTextRsrc( lua_State* );
        int jade_gui_textrsrc_pointSize( lua_State* );
        int jade_gui_textrsrc_string( lua_State* );
        int jade_gui_textrsrc_font( lua_State* );
        int jade_gui_textrsrc_color( lua_State* );
        int jade_gui_textrsrc_maxDimensions( lua_State* );
        int jade_gui_textrsrc_baseline( lua_State* );
        // int jade_gui_textrsrc_hinting( lua_State* );
        // int jade_gui_textrsrc_antialiasing( lua_State* );
        int jade_gui_textrsrc_tostring( lua_State* );
        
        int jade_gui_newImageRsrc( lua_State* );
        
        int jade_gui_newGroup( lua_State* );                                    // New group with positition 0,0 and dimensions 1,1
        int jade_gui_group_addElement( lua_State* );
        int jade_gui_group_removeElement( lua_State* );
        int jade_gui_group_drawBackground( lua_State* );
        int jade_gui_group_setShownCallback( lua_State* );
        int jade_gui_group_setHiddenCallback( lua_State* );
        int jade_gui_group_setClosedCallback( lua_State* );
        int jade_gui_group_tostring( lua_State* );
        
        int jade_gui_newButton( lua_State* );
        int jade_gui_button_setContents( lua_State* );
        int jade_gui_button_setToggleOnCallback( lua_State* );                  // Sets a Lua function or closure as the toggle-on callback
        int jade_gui_button_setToggleOffCallback( lua_State* );                 // Sets a Lua function or closure as the toggle-off callback
        int jade_gui_button_tostring( lua_State* );
        
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
        
        int jade_windowsys_newWindow( lua_State* );                             // Creates a new window; note that windows do not use Lua's GC
        int jade_windowsys_window_getTopGroup( lua_State* );
        // int jade_windowsys_window_setFullscreen( lua_State* );
        int jade_windowsys_window_setTitle( lua_State* );
        int jade_windowsys_window_requestRedraw( lua_State* );
        int jade_windowsys_window_gc( lua_State* );
        int jade_windowsys_window_toString( lua_State* );
    }
}

/******************************************************************************//******************************************************************************/

#endif


