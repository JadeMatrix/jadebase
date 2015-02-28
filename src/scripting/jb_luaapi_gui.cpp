/* 
 * jb_luaapi_gui.cpp
 * 
 * Implements GUI API fron jb_luaapi.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_luaapi.hpp"

#include "../jadebase.hpp"                                                      // Include everything

/******************************************************************************//******************************************************************************/

namespace jade
{
    void group_to_udata( lua_State* state, jade::group* g )                     // Pushes onto stack
    {
        jade::group** group_p = ( jade::group** )lua_newuserdata( state, sizeof( jade::group* ) );
        
        ( *group_p ) = g;
        
        lua_newtable( state );
        {
            lua_pushcfunction( state, jade::lua::jade_gui_element_position );
            lua_setfield( state, -2, "position" );
            lua_pushcfunction( state, jade::lua::jade_gui_element_dimensions );
            lua_setfield( state, -2, "dimensions" );
            
            lua_pushcfunction( state, jade::lua::jade_gui_group_addElement );
            lua_setfield( state, -2, "add_element" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_removeElement );
            lua_setfield( state, -2, "remove_element" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_drawBackground );
            lua_setfield( state, -2, "draw_background" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_setShownCallback );
            lua_setfield( state, -2, "shown_callback" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_setHiddenCallback );
            lua_setfield( state, -2, "hidden_callback" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_setClosedCallback );
            lua_setfield( state, -2, "closed_callback" );
            // lua_pushcfunction( state, jade::lua::jade_gui_group_gc );
            // lua_setfield( state, -2, "__gc" );
            lua_pushcfunction( state, jade::lua::jade_gui_group_tostring );
            lua_setfield( state, -2, "__tostring" );
            
            lua_pushnumber( state, jade::JADE_GROUP );
            lua_setfield( state, -2, "__type_key" );
            
            lua_pushstring( state, "Edit jb_luaapi.cpp to change group's metatable" );
            lua_setfield( state, -2, "__metatable" );                           // Protect metatable
            
            lua_pushstring( state, "__index" );                                 // Create object index
            lua_pushvalue( state, -2 );
            lua_settable( state, -3 );
        }
        lua_setmetatable( state, -2 );
    }
    
    namespace lua
    {
        int jade_gui_element_position( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc < 1
                    || !check_udata_type( state, 1, JADE_PNG_FILE ) )
                {
                    luaL_error( state, "Call of element:position() on a non-element type" );
                    return 0;
                }
                
                gui_element* element = *( gui_element** )lua_touserdata( state, 1 );
                
                switch( argc )
                {
                case 3:
                    {
                        if( lua_isnumber( state, 2 )
                            && lua_isnumber( state, 3 ) )
                        {
                            element -> setRealPosition( lua_tonumber( state, 2 ),
                                                        lua_tonumber( state, 3 ) );
                        }
                        else
                        {
                            luaL_error( state, "'x' (1) or 'y' (2) not a number for element:position()" );
                            return 0;
                        }
                    }
                case 1:
                    {
                        std::pair< int, int > pos( element -> getRealPosition() );
                        lua_pushnumber( state, pos.first );
                        lua_pushnumber( state, pos.second );
                    }
                    break;
                default:
                    luaL_error( state, "element:position() requires exactly 0 or 2 arguments" );
                    return 0;
                }
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_element_dimensions( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_gui_resource_dimensions( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_gui_newTextRsrc( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                text_rsrc** rsrc_p = ( text_rsrc** )lua_newuserdata( state, sizeof( text_rsrc* ) );
                
                switch( lua_gettop( state ) )
                {
                case 0:
                    ( *rsrc_p ) = new text_rsrc();                           // Default values
                    break;
                case 3:
                    if( !lua_isnumber( state, 1 ) )
                    {
                        luaL_error( state, "'size' (1) not a number for new_text_rsrc()" );
                        lua_pop( state, 1 );
                        return 0;
                    }
                    if( !lua_isstring( state, 2 ) )
                    {
                        luaL_error( state, "'font' (2) not a string for new_text_rsrc()" );
                        lua_pop( state, 1 );
                        return 0;
                    }
                    if( !lua_isstring( state, 3 ) )
                    {
                        luaL_error( state, "'string' (3) not a string for new_text_rsrc()" );
                        lua_pop( state, 1 );
                        return 0;
                    }
                    
                    ( *rsrc_p ) = new text_rsrc( lua_tonumber( state, 1 ),
                                                 lua_tostring( state, 2 ),
                                                 lua_tostring( state, 3 ) );
                    break;
                default:
                    luaL_error( state, "new_text_rsrc() requires exactly 0 or 3 arguments" );
                    lua_pop( state, 1 );
                    return 0;
                    break;
                }
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_textrsrc_pointSize );
                    lua_setfield( state, -2, "point_size" );
                    lua_pushcfunction( state, jade_gui_textrsrc_string );
                    lua_setfield( state, -2, "string" );
                    lua_pushcfunction( state, jade_gui_textrsrc_font );
                    lua_setfield( state, -2, "font" );
                    lua_pushcfunction( state, jade_gui_textrsrc_color );
                    lua_setfield( state, -2, "color" );
                    lua_pushcfunction( state, jade_gui_textrsrc_maxDimensions );
                    lua_setfield( state, -2, "max_dimensions" );
                    lua_pushcfunction( state, jade_gui_textrsrc_baseline );
                    lua_setfield( state, -2, "baseline" );
                    // lua_pushcfunction( state, jade_gui_textrsrc_gc);
                    // lua_setfield( state, -2, "__gc" );
                    lua_pushcfunction( state, jade_gui_textrsrc_tostring );
                    lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_TEXT_RSRC );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change text_rsrc's metatable" );
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
        int jade_gui_textrsrc_pointSize( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_string( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_font( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_color( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 4;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_maxDimensions( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 2;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_textrsrc_baseline( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc <= 2 )
                {
                    if( argc < 1
                        || !check_udata_type( state, 1, JADE_TEXT_RSRC ) )
                    {
                        luaL_error( state, "Call of text_rsrc:baseline() on non-text_rsrc type" );
                        return 0;
                    }
                    
                    text_rsrc* rsrc_p = *( text_rsrc** )lua_touserdata( state, 1 );
                    
                    if( argc == 2 )
                    {
                        if( !lua_isboolean( state, 2 ) )
                        {
                            luaL_error( state, "'enable' (1) not a boolean for text_rsrc:baseline()" );
                            return 0;
                        }
                        
                        rsrc_p -> setEnableBaseline( lua_toboolean( state, 2 ) );
                    }
                    
                    lua_pushboolean( state, rsrc_p -> getEnableBaseline() );
                    
                    return 1;
                }
                else
                {
                    luaL_error( state, "text_rsrc:baseline() requires exactly 0 or 1 arguments" );
                    return 0;
                }
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        // int jade_gui_textrsrc_hinting( lua_State* state )
        // int jade_gui_textrsrc_antialiasing( lua_State* state )
        int jade_gui_textrsrc_tostring( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_gui_newImageRsrc( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_gui_newGroup( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 4 )
                {
                    luaL_error( state, "new_group() requires exactly 4 arguments" );
                    return 0;
                }
                
                if( !lua_isnumber( state, 1 )
                    || !lua_isnumber( state, 2 )
                    || !lua_isnumber( state, 3 )
                    || !lua_isnumber( state, 4 ) )
                {
                    luaL_error( state, "All arguments to new_group() must be numbers" );
                    return 0;
                }
                
                group_to_udata( state, new group( NULL,
                                                  lua_tonumber( state, 1 ),
                                                  lua_tonumber( state, 2 ),
                                                  lua_tonumber( state, 3 ),
                                                  lua_tonumber( state, 4 ) ) ); // Round down
                
                return 1;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_addElement( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if( lua_gettop( state ) != 2 )
                {
                    luaL_error( state, "group:add_element() requires exactly 1 arguments" );
                    return 0;
                }
                
                if( !check_udata_type( state, 1, JADE_GROUP ) )
                {
                    luaL_error( state, "Call of group:add_element() on a non-group type" );
                    return 0;
                }
                
                if( lua_getmetatable( state, 2 ) )                              // Check if the argument is a gui_element
                {
                    lua_getfield( state, -1, "__type_key" );
                    
                    if( lua_isnumber( state, -1 ) )
                    {
                        switch( ( int )lua_tonumber( state, -1 ) )              // TODO: add additional types
                        {
                        case JADE_GROUP:
                        case JADE_BUTTON:
                            {
                                group* parent = *( group** )lua_touserdata( state, 1 );
                                gui_element* element = *( gui_element** )lua_touserdata( state, 2 );
                                parent -> addElement( element );
                            }
                            // ( *( group** )lua_touserdata( state, 1 ) ) -> addElement( *( gui_element** )lua_touserdata( state, 2 ) );
                            return 0;
                        default:
                            break;
                        }
                    }
                }
                
                luaL_error( state, "'element' (1) is not an element for group:add_element()" );
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_removeElement( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_drawBackground( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setShownCallback( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setHiddenCallback( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_group_setClosedCallback( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        // int jade_gui_group_gc( lua_State* state )
        int jade_gui_group_tostring( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        
        int jade_gui_newButton( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int args[] = { 0,
                               0,
                               BUTTON_MIN_WIDTH,
                               BUTTON_MIN_HEIGHT };
                
                switch( lua_gettop( state ) )
                {
                case 4:
                    if( !lua_isnumber( state, 4 ) )
                    {
                        luaL_error( state, "'height' (4) not a number for new_button()" );
                        return 0;
                    }
                    args[ 3 ] = lua_tonumber( state, 4 );                       // Round down
                case 3:
                    if( !lua_isnumber( state, 3 ) )
                    {
                        luaL_error( state, "'width' (3) not a number for new_button()" );
                        return 0;
                    }
                    args[ 2 ] = lua_tonumber( state, 3 );
                case 2:
                    if( !lua_isnumber( state, 2 ) )
                    {
                        luaL_error( state, "'y' (2) not a number for new_button()" );
                        return 0;
                    }
                    args[ 1 ] = lua_tonumber( state, 2 );
                case 1:
                    if( !lua_isnumber( state, 1 ) )
                    {
                        luaL_error( state, "'x' (1) not a number for new_button()" );
                        return 0;
                    }
                    args[ 0 ] = lua_tonumber( state, 1 );
                    break;
                case 0:
                    break;
                default:
                    luaL_error( state, "new_button() requires 0-4 arguments" );
                    return 0;
                }
                
                button** button_p = ( button** )lua_newuserdata( state, sizeof( button* ) );
                
                ( *button_p ) = new button( NULL,
                                            args[ 0 ],
                                            args[ 1 ],
                                            args[ 2 ],
                                            args[ 3 ] );
                
                lua_newtable( state );
                {
                    lua_pushcfunction( state, jade_gui_element_position );
                    lua_setfield( state, -2, "position" );
                    lua_pushcfunction( state, jade_gui_element_dimensions );
                    lua_setfield( state, -2, "dimensions" );
                    
                    lua_pushcfunction( state, jade_gui_button_setContents );
                    lua_setfield( state, -2, "contents" );
                    lua_pushcfunction( state, jade_gui_button_setToggleOnCallback );
                    lua_setfield( state, -2, "toggle_on_callback" );
                    lua_pushcfunction( state, jade_gui_button_setToggleOffCallback );
                    lua_setfield( state, -2, "toggle_off_callback" );
                    // lua_pushcfunction( state, jade_gui_button_gc );
                    // lua_setfield( state, -2, "__gc" );
                    // lua_pushcfunction( state, jade_gui_button_tostring );
                    // lua_setfield( state, -2, "__tostring" );
                    
                    lua_pushnumber( state, JADE_BUTTON );
                    lua_setfield( state, -2, "__type_key" );
                    
                    lua_pushstring( state, "Edit jb_luaapi.cpp to change button's metatable" );
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
        int jade_gui_button_setContents( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // int argc = lua_gettop( state );
                
                // if( argc > 2 )
                // {
                //     luaL_error( state, "button:set_contents() requires exactly 1 arguments" );
                //     return 0;
                // }
                
                // if( argc < 1
                //     || !check_udata_type( state, 1, JADE_BUTTON ) )
                // {
                //     luaL_error( state, "Call of button:set_contents() on a non-button type" );
                //     return 0;
                // }
                
                // if( lua_getmetatable( state, 2 ) )                              // Check if the argument is a gui resource
                // {
                //     lua_getfield( state, -1, "__type_key" );
                    
                //     if( lua_isnumber( state, -1 ) )
                //     {
                //         switch( ( int )lua_tonumber( state, -1 ) )
                //         {
                //         case JADE_IMAGE_RSRC:
                //         case JADE_TEXT_RSRC:
                //             {
                //                 button* button_p = *( button** )lua_touserdata( state, 1 );
                //                 gui_resource* rsrc = *( gui_resource** )lua_touserdata( state, 2 );
                //                 button_p -> setContents( rsrc );
                //             }
                //             break;
                //         default:
                //             luaL_error( state, "'resource' (1) not a gui_resource for button:set_contents()" );
                //             break;
                //         }
                //     }
                // }
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_setToggleOnCallback( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        int jade_gui_button_setToggleOffCallback( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
        // int jade_gui_button_gc( lua_State* state )
        int jade_gui_button_tostring( lua_State* state ) // TODO:
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
                
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


