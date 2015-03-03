/* 
 * jb_luaapi.cpp
 * 
 * Implements initLuaAPI() from jb_lua.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <vector>

#include "../jadebase.hpp"                                                      // Include everything

/******************************************************************************//******************************************************************************/

namespace jade
{
    // LUA_CALLBACK ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
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
    
    // UTILITY /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    namespace lua
    {
        bool check_udata_type( lua_State* state, int index, udata_type type )
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
        
        std::string err_argcount( std::string func_name, std::string obj_name, int n, ... )
        {
            std::string s;
            
            ff::write( s,
                       obj_name,
                       obj_name.length() ? ":" : "",
                       func_name,
                       "() takes " );
            
            va_list vl;
            va_start( vl, n );
            
            int first = 0;                                                      // Initialize to non-1 value
            
            switch( n )
            {
            case 0:
                ff::write( s, "no" );
                break;
            case 1:
                first = va_arg( vl, int );
                ff::write( s,
                           "exactly ",
                           first );
                break;
            default:
                {
                    std::vector< int > int_args;                                // Vector so we can look ahead
                    
                    for( int i = 0; i < n; ++i )
                        int_args.push_back( va_arg( vl, int ) );
                    
                    for( int i = 0; i < n; ++i )
                    {
                        if( i + 1 < n )                                         // Not the last one
                        {
                            if( int_args[ i + 1 ] < 0 )                         // Next one is negative
                            {
                                ff::write( s,
                                           i == 0 ? "" : ( i + 2 >= n ? ", " : ", or " ) );
                                
                                ff::write( s,
                                           int_args[ i ],
                                           int_args[ i + 1 ] );                 // Includes dash
                                
                                ++i;                                            // Increment because we used two
                            }
                            else
                            {
                                ff::write( s,
                                           i == 0 ? "" : ", ",
                                           int_args[ i ] );
                            }
                        }
                        else
                        {
                            ff::write( s,
                                       i == 0 ? "" : ", or ",
                                       int_args[ i ] );
                        }
                    }
                }
                break;
            }
            
            ff::write( s,
                       " argument",
                       first == 1 ? "" : "s" );
            
            return s;
        }
        std::string err_argtype( std::string func_name, std::string obj_name, std::string arg_name, int index, std::string type_name )
        {
            std::string s;
            
            ff::write( s,
                       "'",
                       arg_name,
                       "' (",
                       index,
                       ") not a ",
                       type_name,
                       " for ",
                       obj_name,
                       obj_name.length() ? ":" : "",
                       func_name,
                       "()" );
            
            return s;
        }
        std::string err_objtype( std::string func_name, std::string obj_name )
        {
            std::string s;
            
            ff::write( s,
                       "Call of ",
                       obj_name,
                       ":",
                       func_name,
                       "() on non-",
                       obj_name,
                       " type" );
            
            return s;
        }
        std::string warn_metatable( std::string file_name, std::string obj_name )
        {
            std::string s;
            
            ff::write( s,
                       "Edit '",
                       file_name,
                       "' to change ",
                       obj_name,
                       "'s metatable" );
            
            return s;
        }
    }
}


