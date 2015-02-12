#ifndef BQT_SETTINGS_HPP
#define BQT_SETTINGS_HPP

/* 
 * bqt_settings.hpp
 * 
 * Subsystem for loading, saving, and retrieving generic settings for use during
 * runtime
 * 
 * Settings files are UTF-8 text files, usually with the suffix .CFG.  Settings
 * are stored one per line; first the key (alphanumeric, underscores, dashes)
 * without quotes, followed by whitespace, followed by the value.  Numerical
 * values can be any decimal format (recognizable by strtod()), string values
 * must be enclosed by double quotes (\", \\ permissable), boolean values are
 * either "true" or "false" (without quotes).  For example:
 *     setting_1 -3.47e1
 *     setting_2 "this is a \"string\""
 *     setting_3 false
 *   EOF
 * Blank lines are skipped.
 * 
 * Reasonable effort has been made to make the the retrieving of settings
 * efficient.  However, the getSetting_* functions should be used wisely.  For
 * example, if a setting is needed in iterating over a large array of items,
 * it would be better to retrieve the setting once at the beginning rather than
 * each time.  The setting is unlikely to change, and if it did, it is probably
 * not a good idea for it to change in the middle of an operation that should
 * SEEM atomic.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <string>

/******************************************************************************//******************************************************************************/

namespace bqt
{
    void loadSettingsFile( std::string file,
                           bool save = false,
                           bool override = true );                              // Load a settings file & add parameters to loaded parameters.  If override is
                                                                                // true, conflicting settings will be overridden; if false, the old ones will be
                                                                                // kept.  If save, loaded settings (not including non-overridden) will be saved
                                                                                // on calls to saveSettings().
                                                                                // Throws an exception and does not load any settings if the file is not in a
                                                                                // known format.
    
    void reloadSettingsFiles( bool flush = true );                              // Reload all loaded settings files.  If flush is true, flushes loaded settings
                                                                                // first.
    
    void saveSettings( std::string file );                                      // Save all loaded settings to a file (overwrites contents).  Settings are saved
                                                                                // in alphabetical order (Unicode-order) for ease of use.
    
    // For the setSetting() functions, if save is true then a call to
    // saveSettings() will write that setting to a file
    void setSetting( std::string key,      double numval, bool save = true );   // Add a number setting
    void setSetting( std::string key, std::string strval, bool save = true );   // Add a string setting
    void setSetting( std::string key,        bool blnval, bool save = true );   // Add a boolean setting
    
    // The following getSetting_* functions throw exceptions if (and only if)
    // an associated value of the requested type does not exist (may be
    // different or may not exist).
    double      getSetting_num( std::string key );                              // Retrieve setting as a double
    std::string getSetting_str( std::string key );                              // Retrieve setting as a string
    bool        getSetting_bln( std::string key );                              // Retrieve setting as a boolean
}

/******************************************************************************//******************************************************************************/

#endif


