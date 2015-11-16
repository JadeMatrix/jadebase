/* 
 * jb_settings.cpp
 * 
 * Implements settings functionality from jb_settings.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_settings.hpp"

#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include "jb_exception.hpp"
#include "jb_launchargs.hpp"
#include "jb_log.hpp"
#include "../threading/jb_mutex.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex settings_mutex;
    
    std::map< std::string, bool > loaded_files;
    
    std::map< std::string, std::pair< double     , bool > > settings_num;
    std::map< std::string, std::pair< std::string, bool > > settings_str;
    std::map< std::string, std::pair< bool       , bool > > settings_bln;
}

/* jb_settings.hpp ************************************************************//******************************************************************************/

namespace jade
{
    void loadSettingsFile( std::string file, bool save, bool override )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        std::map< std::string, std::pair< double     , bool > > temp_num;
        std::map< std::string, std::pair< std::string, bool > > temp_str;
        std::map< std::string, std::pair< bool       , bool > > temp_bln;
        
        std::fstream settings_file;
        
        settings_file.open( file.c_str(), std::fstream::in );
        
        if( !settings_file.is_open() )
        {
            if( save )                                                          // Force open (create) if it's a saveable
                // settings_file.open( file.c_str(), std::fstream::in | std::fstream::out | std::fstream::trunc );
                settings_file.open( file.c_str(), std::fstream::in | std::fstream::out );
            else
            {
                exception e;
                ff::write( *e,
                           "loadSettingsFile(): Could not load file \"",
                           file,
                           "\"" );
                throw e;
            }
        }
        
        ff::write( jb_out,
                   "Loading settings from \"",
                   file,
                   save ? "\" (save)" : "\"",
                   override ? " (override)\n" : "\n" );
        
        std::string got_line;
        
        while( !settings_file.eof() )
        {
            std::getline( settings_file, got_line );
            std::stringstream line_stream( got_line );
            
            std::string key;
            double      numval;
            std::string strval;
            bool        blnval;
            
            line_stream >> std::ws >> key;
            
            if( line_stream.fail() )
                continue;                                                       // Blank line
            
            line_stream >> std::ws;
            std::streampos line_pos = line_stream.tellg();                      // Save start of value in line
            
            line_stream >> numval;
            
            if( !line_stream.fail() )
            {
                temp_num[ key ] = std::pair< double, bool >( numval, save );
                
                if( getDevMode() )
                    ff::write( jb_out,
                               "Loaded setting: ",
                               key,
                               " ",
                               numval,
                               save ? " (saved)\n" : "\n" );
            }
            else
            {
                line_stream.clear();
                line_stream.seekg( line_pos );                                  // Reset...
                line_stream >> std::boolalpha >> blnval;                        // ... and try agian as a bool
                
                if( !line_stream.fail() )
                {
                    temp_bln[ key ] = std::pair< bool, bool >( blnval, save );
                    
                    if( getDevMode() )
                        ff::write( jb_out,
                                   "Loaded setting: ",
                                   key,
                                   " ",
                                   blnval,
                                   save ? " (saved)\n" : "\n" );
                }
                else
                {
                    line_stream.clear();
                    line_stream.seekg( line_pos );                              // Reset and try again as a string
                    
                    char got_c;
                    enum
                    {
                        START,
                        GETTING,
                        ESCAPED,
                        ENDED
                    } string_state = START;
                    
                    while( ( got_c = line_stream.get() ) != EOF )
                    {
                        switch( got_c )
                        {
                        case '\\':
                            {
                                switch( string_state )
                                {
                                case START:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   // "loadSettingsFile(): Starting escape sequence for key \"",
                                                   "loadSettingsFile(): Unknown value format for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                case GETTING:
                                    string_state = ESCAPED;
                                    break;
                                case ESCAPED:
                                    strval += '\\';
                                    string_state = GETTING;
                                    break;
                                case ENDED:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   "loadSettingsFile(): Trailing escape sequence after string value for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                }
                            }
                            break;
                        case '\"':
                            {
                                switch( string_state )
                                {
                                case START:
                                    string_state = GETTING;
                                    break;
                                case GETTING:
                                    string_state = ENDED;
                                    break;
                                case ESCAPED:
                                    strval += got_c;
                                    string_state = GETTING;
                                    break;
                                case ENDED:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   "loadSettingsFile(): Extra \" on string value for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                }
                            }
                            break;
                        default:
                            {
                                switch( string_state )
                                {
                                case START:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   "loadSettingsFile(): Unknown value format for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                case GETTING:
                                    strval += got_c;
                                    break;
                                case ESCAPED:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   "loadSettingsFile(): Unknown escape sequence \"\\",
                                                   ff::ch( got_c ),
                                                   "\" in string value for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                case ENDED:
                                    {
                                        exception e;
                                        ff::write( *e,
                                                   "loadSettingsFile(): Trailing characters on string value for key \"",
                                                   key,
                                                   "\" in file \"",
                                                   file,
                                                   "\"" );
                                        throw e;
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    
                    if( string_state != ENDED )
                    {
                        exception e;
                        ff::write( *e,
                                   "loadSettingsFile(): Failed to parse value for key \"",
                                   key,
                                   "\" in file \"",
                                   file,
                                   "\"" );
                        throw e;
                    }
                    
                    temp_str[ key ] = std::pair< std::string, bool >( strval, save );
                    
                    if( getDevMode() )
                        ff::write( jb_out,
                                   "Loaded setting: ",
                                   key,
                                   " \"",
                                   strval,
                                   save ? "\" (saved)\n" : "\"\n" );
                }
            }
        }
        
        // If we've gotten here, there were no errors reading the settings file
        // so we can go ahead and write them to the subsystem memory.
        
        loaded_files[ file ] = save;
        
        for( std::map< std::string, std::pair< double, bool > >::iterator iter = temp_num.begin();
             iter != temp_num.end();
             ++iter )
        {
            if( override || !settings_num.count( iter -> first ) )
                settings_num[ iter -> first ] = iter -> second;
        }
        for( std::map< std::string, std::pair< std::string, bool > >::iterator iter = temp_str.begin();
             iter != temp_str.end();
             ++iter )
        {
            if( override || !settings_str.count( iter -> first ) )
                settings_str[ iter -> first ] = iter -> second;
        }
        for( std::map< std::string, std::pair< bool, bool > >::iterator iter = temp_bln.begin();
             iter != temp_bln.end();
             ++iter )
        {
            if( override || !settings_bln.count( iter -> first ) )
                settings_bln[ iter -> first ] = iter -> second;
        }
    }
    
    void reloadSettingsFiles( bool flush )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( flush )
        {
            settings_num.clear();
            settings_str.clear();
            settings_bln.clear();
        }
        
        for( std::map< std::string, bool >::iterator iter = loaded_files.begin();
             iter != loaded_files.end();
             ++iter )
        {
            loadSettingsFile( iter -> first, iter -> second, !flush );          // override = !flush: if we flushed, we don't need to override (and may be able
                                                                                // to get a slight speedup); if we didn't flush, we want to override conflicting
                                                                                // settings.
        }
    }
    
    void saveSettings( std::string file )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        std::fstream settings_file( file.c_str(), std::fstream::out | std::fstream::trunc );
        
        if( !settings_file.is_open() )
        {
            exception e;
            ff::write( *e,
                       "saveSettings(): Could not open file \"",
                       file,
                       "\"" );
            throw e;
        }
        
        ff::write( jb_out,
                   "Saving settings to \"",
                   file,
                   "\"\n" );
        
        std::map< std::string, std::pair< double     , bool > >::iterator num_iter = settings_num.begin();
        std::map< std::string, std::pair< std::string, bool > >::iterator str_iter = settings_str.begin();
        std::map< std::string, std::pair< bool       , bool > >::iterator bln_iter = settings_bln.begin();
        
        for( int i = settings_num.size() + settings_str.size() + settings_bln.size();
             i > 0;
             --i )
        {                                                                       // We ensure we never have duplicate keys across the three containers, so
                                                                                // checking each key individually (instead of in a tree) should be safe.
            if( num_iter != settings_num.end()
                && ( str_iter == settings_str.end() || num_iter -> first < str_iter -> first )
                && ( bln_iter == settings_bln.end() || num_iter -> first < bln_iter -> first ) )
            {
                if( num_iter -> second.second )
                {
                    settings_file << num_iter -> first
                                  << ' '
                                  << num_iter -> second.first
                                  << '\n';
                }
                
                ++num_iter;
                
                continue;
            }
            
            if( str_iter != settings_str.end()
                && ( num_iter == settings_num.end() || str_iter -> first < num_iter -> first )
                && ( bln_iter == settings_bln.end() || str_iter -> first < bln_iter -> first ) )
            {
                if( str_iter -> second.second )
                {
                    std::string& strval( str_iter -> second.first );
                    
                    settings_file << str_iter -> first
                                  << " \"";
                    
                    for( int i = 0; i < strval.size(); ++i )
                    {
                        if( strval[ i ] == '\\' )
                            settings_file << "\\\\";
                        else
                            settings_file << strval[ i ];
                    }
                    
                    settings_file << "\"\n";
                }
                
                ++str_iter;
                
                continue;
            }
            
            if( bln_iter != settings_bln.end()
                && ( num_iter == settings_num.end() || bln_iter -> first < num_iter -> first )
                && ( str_iter == settings_str.end() || bln_iter -> first < str_iter -> first ) )
            {
                if( bln_iter -> second.second )
                {
                    settings_file << bln_iter -> first
                                  << ' '
                                  << bln_iter -> second.first
                                  << '\n';
                }
                
                ++bln_iter;
                
                continue;
            }
            
            exception e;
            ff::write( *e,
                       "saveSettings(): Failed to save a setting to file \"",
                       file,
                       "\"" );
            throw e;                                                            // Skipped if one of the above conditions passes
        }
    }
    
    void setSetting( std::string key, double numval, bool save )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( !settings_str.erase( key ) )                                        // Erase any settings with key with other value types
            settings_bln.erase( key );
        
        settings_num[ key ].first  = numval;
        settings_num[ key ].second = save;
    }
    void setSetting( std::string key, std::string strval, bool save )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( !settings_num.erase( key ) )                                        // Erase any settings with key with other value types
            settings_bln.erase( key );
        
        settings_str[ key ].first  = strval;
        settings_str[ key ].second = save;
    }
    void setSetting( std::string key, bool blnval, bool save )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( !settings_str.erase( key ) )                                        // Erase any settings with key with other value types
            settings_str.erase( key );
        
        settings_bln[ key ].first  = blnval;
        settings_bln[ key ].second = save;
    }
    
    double getSetting_num( std::string key )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( settings_num.count( key ) )
            return settings_num[ key ].first;
        else
        {
            exception e;
            ff::write( *e,
                       "getSetting_num(): No numerical value for setting \"",
                       key,
                       "\"" );
            throw e;
        }
    }
    std::string getSetting_str( std::string key )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( settings_str.count( key ) )
            return settings_str[ key ].first;
        else
        {
            exception e;
            ff::write( *e,
                       "getSetting_num(): No string value for setting \"",
                       key,
                       "\"" );
            throw e;
        }
    }
    bool getSetting_bln( std::string key )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        if( settings_bln.count( key ) )
            return settings_bln[ key ].first;
        else
        {
            exception e;
            ff::write( *e,
                       "getSetting_num(): No boolean value for setting \"",
                       key,
                       "\"" );
            throw e;
        }
    }
    
    bool getSetting( std::string key, double& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder = settings_num.find( key );
        
        if( finder != settings_num.end() )
        {
            val = finder -> second.first;
            return true;
        }
        else
            return false;
    }
    bool getSetting( std::string key, std::string& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder = settings_str.find( key );
        
        if( finder != settings_str.end() )
        {
            val = finder -> second.first;
            return true;
        }
        else
            return false;
    }
    bool getSetting( std::string key, bool& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder = settings_bln.find( key );
        
        if( finder != settings_bln.end() )
        {
            val = finder -> second.first;
            return true;
        }
        else
            return false;
    }
    
    void coerceSetting( std::string key, double& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder_num = settings_num.find( key );
        
        if( finder_num != settings_num.end() )
            val = finder_num -> second.first;
        else
        {
            auto finder_str = settings_str.find( key );
            
            if( finder_str != settings_str.end() )
            {
                std::stringstream ns( finder_str -> second.first );
                
                ns >> val;
                
                if( ns.fail() )
                    val = NAN;
            }
            else
            {
                auto finder_bln = settings_bln.find( key );
                
                if( finder_bln != settings_bln.end() )
                    val = finder_bln -> second.first ? 1 : 0;
                else
                {
                    exception e;
                    ff::write( *e,
                               "coerceSetting() number: No value to coerce for setting \"",
                               key,
                               "\"" );
                    throw e;
                }
            }
        }
    }
    void coerceSetting( std::string key, std::string& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder_str = settings_str.find( key );
        
        if( finder_str != settings_str.end() )
        {
            val = finder_str -> second.first;
        }
        else
        {
            auto finder_bln = settings_bln.find( key );
            
            if( finder_bln != settings_bln.end() )
            {
                val.clear();
                ff::write( val, finder_bln -> second.first );
            }
            else
            {
                auto finder_num = settings_num.find( key );
                
                if( finder_num != settings_num.end() )
                {
                    val.clear();
                    ff::write( val, finder_num -> second.first );
                }
                else
                {
                    exception e;
                    ff::write( *e,
                               "coerceSetting() string: No value to coerce for setting \"",
                               key,
                               "\"" );
                    throw e;
                }
            }
        }
    }
    void coerceSetting( std::string key, bool& val )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        auto finder_bln = settings_bln.find( key );
        
        if( finder_bln != settings_bln.end() )
            val = finder_bln -> second.first;
        else
        {
            auto finder_num = settings_num.find( key );
            
            if( finder_num != settings_num.end() )
                val = finder_num -> second.first != 0.0;
            else
            {
                auto finder_str = settings_str.find( key );
                
                if( finder_str != settings_str.end() )
                {
                    std::stringstream bs( finder_str -> second.first );
                    
                    bs >> std::boolalpha >> val;
                    
                    if( bs.fail() )
                        val = ( finder_str -> second.first != "" );
                }
                else
                {
                    exception e;
                    ff::write( *e,
                               "coerceSetting() bool: No value to coerce for setting \"",
                               key,
                               "\"" );
                    throw e;
                }
            }
        }
    }
}

/* jb_luaapi.hpp **************************************************************//******************************************************************************/

#include "../scripting/jb_luaapi.hpp"

namespace jade
{
    namespace lua
    {
        int jade_util_getSetting( lua_State* state )
        {
            LUA_API_SAFETY_BLOCK_BEGIN
            {///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int argc = lua_gettop( state );
                
                if( argc != 1 )
                {
                    luaL_error( state, err_argcount( "get_setting", "", 1, 1 ).c_str() );
                    return 0;
                }
                
                if( !lua_isstring( state, 1 ) )
                {
                    luaL_error( state, err_argtype( "get_setting", "", "key", 1, "string" ).c_str() );
                    return 0;
                }
                
                const char* key_str = lua_tostring( state, 1 );
                
                double      val_num;
                std::string val_str;
                bool        val_bln;
                
                if( getSetting( key_str, val_num ) )
                {
                    lua_pushnumber( state, val_num );
                    return 1;
                }
                if( getSetting( key_str, val_str ) )
                {
                    lua_pushstring( state, val_str.c_str() );
                    return 1;
                }
                if( getSetting( key_str, val_bln ) )
                {
                    lua_pushboolean( state, val_bln );
                    return 1;
                }
                
                std::string noreturn_err;
                ff::write( noreturn_err,
                           "No value associated with setting key \"",
                           key_str,
                           "\"" );
                luaL_error( state, noreturn_err.c_str() );
                return 0;
            }///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            LUA_API_SAFETY_BLOCK_END
        }
    }
}


