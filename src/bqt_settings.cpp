/* 
 * bqt_settings.cpp
 * 
 * Implements settings functionality from bqt_settings.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_settings.hpp"

#include <map>
#include <fstream>
#include <set>
#include <sstream>

#include "threading/bqt_mutex.hpp"
#include "bqt_exception.hpp"
#include "bqt_log.hpp"
#include "bqt_launchargs.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex settings_mutex;
    
    std::map< std::string, bool > loaded_files;
    
    std::map< std::string, std::pair< double     , bool > > settings_num;
    std::map< std::string, std::pair< std::string, bool > > settings_str;
    std::map< std::string, std::pair< bool       , bool > > settings_bln;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void loadSettingsFile( std::string file, bool save, bool override )
    {
        scoped_lock< mutex > slock( settings_mutex );
        
        std::map< std::string, std::pair< double     , bool > > temp_num;
        std::map< std::string, std::pair< std::string, bool > > temp_str;
        std::map< std::string, std::pair< bool       , bool > > temp_bln;
        
        std::fstream settings_file( file.c_str(), std::fstream::in );
        
        if( !settings_file.is_open() )
        {
            exception e;
            ff::write( *e,
                       "loadSettingsFile(): Could not load file \"",
                       file,
                       "\"" );
            throw e;
        }
        
        ff::write( bqt_out,
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
            
            line_stream >> key;
            
            if( line_stream.fail() )
            {
                exception e;
                ff::write( *e,
                           "loadSettingsFile(): Failed to retrieve a settings key from file \"",
                           file,
                           "\"; check file formatting and try again" );
                throw e;
            }
            
            line_stream >> std::ws;
            std::streampos line_pos = line_stream.tellg();                      // Save start of value in line
            
            line_stream >> numval;
            
            if( !line_stream.fail() )
            {
                temp_num[ key ] = std::pair< double, bool >( numval, save );
                
                if( getDevMode() )
                    ff::write( bqt_out,
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
                        ff::write( bqt_out,
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
                    std::getline( line_stream, strval, '\"' );                  // Get rid of the first double quote
                    
                    if( strval.size() == 0 )                                    // aka there are no non-ws characters before the double quote
                    {
                        std::getline( line_stream, strval, '\"' );
                        
                        if( !line_stream.fail() )
                        {
                            temp_str[ key ] = std::pair< std::string, bool >( strval, save );
                            
                            if( getDevMode() )
                                ff::write( bqt_out,
                                           "Loaded setting: ",
                                           key,
                                           " \"",
                                           strval,
                                           save ? "\" (saved)\n" : "\"\n" );
                            
                            goto string_success;
                        }
                    }
                    
                    {
                        exception e;
                        ff::write( *e,
                                   "loadSettingsFile(): Failed to parse settings value for key \"",
                                   key,
                                   "\" from file \"",
                                   file,
                                   "\"" );
                        throw e;
                    }
                    
                string_success:
                    NULL;
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
        
        ff::write( bqt_out,
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
                && num_iter -> first < str_iter -> first
                && num_iter -> first < bln_iter -> first )
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
                && str_iter -> first < num_iter -> first
                && str_iter -> first < bln_iter -> first )
            {
                if( str_iter -> second.second )
                {
                    settings_file << str_iter -> first
                                  << " \""
                                  << str_iter -> second.first
                                  << "\"\n";
                }
                
                ++str_iter;
                
                continue;
            }
            
            if( bln_iter != settings_bln.end()
                && bln_iter -> first < str_iter -> first
                && bln_iter -> first < num_iter -> first )
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
}


