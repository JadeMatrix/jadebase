#ifndef JADEBASE_SETTINGS_HPP
#define JADEBASE_SETTINGS_HPP

/* 
 * jb_settings.hpp
 * 
 * Subsystem for loading, saving, and retrieving generic settings for use during
 * runtime
 * 
 * Settings files are UTF-8 text files, usually with the suffix .CFG.  Settings
 * are stored one per line; first the key (alphanumeric, underscores, dashes)
 * without quotes, followed by whitespace, followed by the value.  Numerical
 * values can be any decimal format (recognizable by strtod()), string values
 * must be enclosed by double quotes (\", \\ permissable), boolean values are
 * those recognizable by std::boolalpha.  For example:
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

namespace jade
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
    
    #define SETSETTING_DEFAULT_SAVEVAL true                                     // For the setSetting() functions, if save is true then a call to
                                                                                // saveSettings() will write that setting to a file
    void setSetting( std::string key,
                          double numval,
                            bool save = SETSETTING_DEFAULT_SAVEVAL );           // Add a number setting
    void setSetting( std::string key,
                     std::string strval,
                            bool save = SETSETTING_DEFAULT_SAVEVAL );           // Add a string setting
    void setSetting( std::string key,
                            bool blnval,
                            bool save = SETSETTING_DEFAULT_SAVEVAL );           // Add a boolean setting
    
    // The following getSetting_* functions throw exceptions if (and only if)
    // an associated value of the requested type does not exist (may be
    // different or may not exist).
    double      getSetting_num( std::string key );                              // Retrieve setting as a double
    std::string getSetting_str( std::string key );                              // Retrieve setting as a string
    bool        getSetting_bln( std::string key );                              // Retrieve setting as a boolean
    
    // The following functions return true if a setting with that key & type
    // exists, and place the value in the supplied reference.  If they key does
    // not point to a setting of that type, they return false and the reference
    // is not used.
    bool getSetting( std::string, double&      );                               // Retrieve a double setting
    bool getSetting( std::string, std::string& );                               // Retrieve a string setting
    bool getSetting( std::string, bool&        );                               // Retrieve a boolean setting
    // The following functions attempt type coercion, and so always place some
    // value in the supplied reference.
    void coerceSetting( std::string, double&      );                            // Retrieve setting as a double
    void coerceSetting( std::string, std::string& );                            // Retrieve setting as a string
    void coerceSetting( std::string, bool&        );                            // Retrieve setting as a boolean
}

/******************************************************************************//******************************************************************************/

#endif


