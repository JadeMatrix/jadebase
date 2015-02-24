/* 
 * jb_luaapi.cpp
 * 
 * Implements initLuaAPI() from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "../jadebase.hpp"                                                      // Include everything

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

/* Template (c&p) for safety blocks
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
*/

namespace
{
    enum luaapi_types
    {
        JADE_PNG_FILE,
        JADE_WINDOW
    };
    
    int jade_util_log_base( lua_State* state, bool dev_log )
    {
        LUA_API_SAFETY_BLOCK_BEGIN
        {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            int argc = lua_gettop( state );
            
            if( !dev_log
                || ( dev_log && jade::getDevMode() ) )                          // Print message
            {
                std::string log_string = "Lua log message: ";
                
                for( int i = 0; i < argc; ++i )
                    ff::write( log_string,
                               luaL_tolstring( state, i + 1, NULL ) );          // Use luaL_tolstring() to get __tostring() conversions
                
                ff::write( jb_out, log_string );
            }
            else
                lua_pop( state, argc );                                         // Ignore & pop all arguments
            
            return 0;
        }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        LUA_API_SAFETY_BLOCK_END
    }
    
    bool check_udata_type( lua_State* state, int index, luaapi_types type )
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

/* jb_luaapi.hpp **************************************************************//******************************************************************************/

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
    
    namespace lua
    {
        // FILETYPES ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_filetypes_png_new( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "png.new() requires exactly 1 argument" );
                    return 0;
                }
                if( !lua_isstring( state, 1 ) )
                {
                    luaL_error( state, "'filename' not a string for png.new()" );
                    return 0;
                }
                
                png_file** file_p = ( png_file** )lua_newuserdata( state, sizeof( png_file* ) );
                
                ( *file_p ) = new png_file( lua_tostring( state, 1 ) );
                
                lua_newtable( state );                                          // Create metatable
                {
                    lua_pushcfunction( state, jade_filetypes_png_getDimensions );
                    lua_setfield( state, -2, "dimensions" );
                    lua_pushcfunction( state, jade_filetypes_png_getBitDepth );
                    lua_setfield( state, -2, "bit_depth" );
                    lua_pushcfunction( state, jade_filetypes_png_getColorType );
                    lua_setfield( state, -2, "color_type" );
                    lua_pushcfunction( state, jade_filetypes_png_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_filetypes_png_toString );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_PNG_FILE );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change png_file's metatable" );
                    lua_setfield( state, -2, "__metatable" );                   // Protect metatable
                    
                    lua_pushstring( state, "__index" );                         // Create object index
                    lua_pushvalue( state, -2 );
                    lua_settable( state, -3 );
                }
                lua_setmetatable( state, -2 );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_getDimensions( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )                                   // First argument in png_file, so no others
                {
                    luaL_error( state, "png:get_dimensions() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_dimensions() on a non-png_file type" );
                    return 0;
                }
                
                std::pair< unsigned int, unsigned int > dimensions = ( *( png_file** )lua_touserdata( state, 1 ) ) -> getDimensions();
                lua_pushnumber( state, dimensions.first );
                lua_pushnumber( state, dimensions.second );
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_getBitDepth( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:get_bit_depth() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_bit_depth() on a non-png_file type" );
                    return 0;
                }
                
                lua_pushnumber( state, ( *( png_file** )lua_touserdata( state, 1 ) ) -> getBitDepth() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_getColorType( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:get_color_type() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:get_color_type() on a non-png_file type" );
                    return 0;
                }
                
                lua_pushnumber( state, ( *( png_file** )lua_touserdata( state, 1 ) ) -> getColorType() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__gc() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:__gc() on a non-png_file type" );
                    return 0;
                }
                
                delete *( png_file** )lua_touserdata( state, 1 );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_filetypes_png_toString( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__tostring() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of png:__tostring() on a non-png_file type" );
                    return 0;
                }
                
                std::string png_string;
                
                ff::write( png_string,
                           "jade::png_file at 0x",
                           ff::to_x( ( long )( *( png_file** )lua_touserdata( state, 1 ) ) ) );
                
                lua_pushstring( state, png_string.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        // GUI /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // MAIN ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // SCRIPTING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // TASKING /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        
        // THREADING ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_util_log( lua_State* state )
        {
            return jade_util_log_base( state, false );
        }
        int jade_util_log_dev( lua_State* state )
        {
            return jade_util_log_base( state, true );
        }
        
        int jade_util_quit( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "quit() requires exactly 0 arguments" );
                else
                {
                    if( getDevMode() )
                        ff::write( jb_out, "Quit requested from Lua script\n" );
                    jb_setQuitFlag();
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_util_loadSettingsFile( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                switch( argc )                                                  // Check arguments
                {
                case 2:
                    if( !lua_isboolean( state, 2 ) )
                    {
                        luaL_error( state, "'override' (2) not a boolean for load_settings_file()" );
                        return 0;
                    }
                case 1:
                    if( !lua_isstring( state, 1 ) )
                    {
                        luaL_error( state, "'file' (1) not a string for load_settings_file()" );
                        return 0;
                    }
                    break;
                default:
                    luaL_error( state, "load_settings_file() requires 1-2 arguments" );
                    return 0;
                }
                
                switch( argc )                                                  // Pass arguments
                {
                case 1:
                    loadSettingsFile( luaL_tolstring( state, 1, NULL ),
                                      false );
                    break;
                case 2:
                    loadSettingsFile( luaL_tolstring( state, 1, NULL ),
                                      false,
                                      lua_toboolean( state, 2 ) );
                    break;
                default:
                    // Won't get here because we check range above
                    break;
                }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_reloadSettingsFiles( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc == 0 )
                    reloadSettingsFiles();
                else
                    if( argc == 1 )
                    {
                        if( lua_isboolean( state, 1 ) )
                            reloadSettingsFiles( lua_toboolean( state, 1 ) );
                        else
                            luaL_error( state, "'flush' (1) not a boolean for reload_settings_files()" );
                    }
                    else
                        luaL_error( state, "reload_settings_files() requires 0-1 arguments" );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_setSetting( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc == 2 )
                {
                    if( !lua_isstring( state, 1 ) )                             // Check 'key' (1) argument
                    {
                        luaL_error( state, "'key' (1) not a string for set_setting()" );
                        return 0;
                    }
                    
                    switch( lua_type( state, 2 ) )                              // Check 'value' 2) argument & execute
                    {
                    case LUA_TNUMBER:
                        setSetting( luaL_tolstring( state, 1, NULL ), lua_tonumber( state, 2 ), true );
                        break;
                    case LUA_TBOOLEAN:
                        setSetting( luaL_tolstring( state, 1, NULL ), ( bool )lua_toboolean( state, 2 ), true );
                        break;
                    case LUA_TSTRING:
                        setSetting( luaL_tolstring( state, 1, NULL ), std::string( luaL_tolstring( state, 2, NULL ) ), true );
                        break;
                    default:
                        luaL_error( state, "'value' (2) not a number, string, or boolean for set_setting()" );
                        return 0;
                    }
                    
                    saveSettings( getUserSettingsFileName() );                  // Save every time a script changes a setting
                }
                else
                    luaL_error( state, "set_setting() requires exactly 2 arguments" );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        // int jade_util_getSetting( lua_State* )                               // Implemented in jb_settings.cpp
        
        int jade_util_getJadebaseVersion( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "get_jadebase_version() requires exactly 0 arguments" );
                else
                {
                    lua_pushstring( state, "jadebase" );
                    
                    version& jadebase_version( getJadebaseVersion() );
                    
                    lua_pushnumber( state, jadebase_version.major );
                    lua_pushnumber( state, jadebase_version.minor );
                    lua_pushnumber( state, jadebase_version.patch );
                }
                
                return 4;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_util_getProgramVersion( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 0 )
                    luaL_error( state, "get_program_version() requires exactly 0 arguments" );
                else
                {
                    lua_pushstring( state, getProgramName().c_str() );
                    
                    version program_version( getProgramVersion() );
                    
                    lua_pushnumber( state, program_version.major );
                    lua_pushnumber( state, program_version.minor );
                    lua_pushnumber( state, program_version.patch );
                }
                
                return 4;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        // WINDOWSYS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        int jade_windowsys_newWindow( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 0 )
                {
                    luaL_error( state, "new_window() requires exactly 0 arguments" );
                    return 0;
                }
                
                window** file_p = ( window** )lua_newuserdata( state, sizeof( window* ) );
                
                ( *file_p ) = new window();                                     // Create window
                submitTask( new window::manipulate( *file_p ) );                // Submit a manipulate
                
                lua_newtable( state );                                          // Create metatable
                {
                    lua_pushcfunction( state, jade_windowsys_window_getTopGroup );
                    lua_setfield( state, -2, "get_top_group" );
                    lua_pushcfunction( state, jade_windowsys_window_setTitle );
                    lua_setfield( state, -2, "set_title" );
                    lua_pushcfunction( state, jade_windowsys_window_requestRedraw );
                    lua_setfield( state, -2, "request_redraw" );
                    lua_pushcfunction( state, jade_windowsys_window_gc );
                    lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_windowsys_window_toString );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_WINDOW );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change window's metatable" );
                    lua_setfield( state, -2, "__metatable" );                   // Protect metatable
                    
                    lua_pushstring( state, "__index" );                         // Create object index
                    lua_pushvalue( state, -2 );
                    lua_settable( state, -3 );
                }
                lua_setmetatable( state, -2 );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_getTopGroup( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // if( lua_gettop( state ) > 1 )
                // {
                //     luaL_error( state, "window:set_title() takes exactly 0 arguments" );
                //     return 0;
                // }
                
                // if( !check_udata_type( state, 1, JADE_WINDOW ) )
                // {
                //     luaL_error( state, "Call of window:get_top_group() on a non-window type" );
                //     return 0;
                // }
                
                // group* g = ( *( window** )lua_touserdata( state, 1 ) ) -> getTopGroup();
                
                luaL_error( state, "window:get_top_group() not yet implemented" );
                return 0;
                
                // return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_setTitle( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 2 )                                  // Object + string, so 2 = 1
                {
                    luaL_error( state, "window:set_title() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:set_title() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:set_title(): Userdata window is NULL" );
                    return 0;
                }
                
                window::manipulate* wm = new window::manipulate( *w );
                
                wm -> setTitle( luaL_tolstring( state, 2, NULL ) );
                
                submitTask( wm );
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_requestRedraw( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "window:request_redraw() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:request_redraw() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:request_redraw(): Userdata window is NULL" );
                    return 0;
                }
                
                ( *w ) -> requestRedraw();
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_gc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) > 1 )
                {
                    luaL_error( state, "png:__gc() requires exactly 0 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:__gc() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:__gc(): Userdata window is NULL" );
                    return 0;
                }
                
                window::manipulate* wm = new window::manipulate( *w );
                
                wm -> close();
                
                submitTask( wm );
                
                ( *w ) = NULL;                                                  // NULL-out userdata
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_windowsys_window_toString( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 1 )
                {
                    luaL_error( state, "window:__tostring() takes exactly 1 argument" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_WINDOW ) )
                {
                    luaL_error( state, "Call of window:__tostring() on a non-window type" );
                    return 0;
                }
                
                window** w = ( window** )lua_touserdata( state, 1 );
                if( *w == NULL )
                {
                    luaL_error( state, "window:__tostring(): Userdata window is NULL" );
                    return 0;
                }
                
                std::string window_string;
                
                ff::write( window_string,
                           "jade::window '",
                           ( *w ) -> getTitle(),
                           "'" );                                               // Don't try to get the platform window for an ID for now, as if new_window()
                                                                                // and window:__tostring() on that window are called in a script executed on the
                                                                                // main thread, the platform window probably will not have been created yet.
                
                lua_pushstring( state, window_string.c_str() );
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}

/* jb_lua.hpp *****************************************************************//******************************************************************************/

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
                // "png" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                lua_newtable( state );
                {
                    // Functions
                    
                    lua_pushcfunction( state, lua::jade_filetypes_png_new );
                    lua_setfield( state, -2, "new" );
                    
                    // Constants
                    
                    lua_pushnumber( state, png_file::GRAY );
                    lua_setfield( state, -2, "GRAY" );
                    lua_pushnumber( state, png_file::PALETTE );
                    lua_setfield( state, -2, "PALETTE" );
                    lua_pushnumber( state, png_file::RGB );
                    lua_setfield( state, -2, "RGB" );
                    lua_pushnumber( state, png_file::RGB_ALPHA );
                    lua_setfield( state, -2, "RGB_ALPHA" );
                    lua_pushnumber( state, png_file::GRAY_ALPHA );
                    lua_setfield( state, -2, "GRAY_ALPHA" );
                }
                lua_setfield( state, -2, "png" );
            }
            lua_setfield( state, -2, "filetypes" );
            
            // "gui" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "gui" );
            
            // "main" //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "scripting" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "tasking" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                
            }
            lua_setfield( state, -2, "tasking" );
            
            // "threading" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // "utility" ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, lua::jade_util_log );
                lua_setfield( state, -2, "log" );
                lua_pushcfunction( state, lua::jade_util_log_dev );
                lua_setfield( state, -2, "log_dev" );
                
                lua_pushcfunction( state, lua::jade_util_quit );
                lua_setfield( state, -2, "quit" );
                
                lua_pushcfunction( state, lua::jade_util_loadSettingsFile );
                lua_setfield( state, -2, "load_settings_file" );
                lua_pushcfunction( state, lua::jade_util_reloadSettingsFiles );
                lua_setfield( state, -2, "reload_settings_files" );
                lua_pushcfunction( state, lua::jade_util_setSetting );
                lua_setfield( state, -2, "set_setting" );
                lua_pushcfunction( state, lua::jade_util_getSetting );
                lua_setfield( state, -2, "get_setting" );
                
                lua_pushcfunction( state, lua::jade_util_getJadebaseVersion );
                lua_setfield( state, -2, "get_jadebase_version" );
                lua_pushcfunction( state, lua::jade_util_getProgramVersion );
                lua_setfield( state, -2, "get_program_version" );
            }
            lua_setfield( state, -2, "utility" );
            
            // "windowsys" /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            lua_newtable( state );
            {
                lua_pushcfunction( state, lua::jade_windowsys_newWindow );
                lua_setfield( state, -2, "new_window" );
            }
            lua_setfield( state, -2, "windowsys" );
        }
        lua_setglobal( state, "jade" );
    }
}


