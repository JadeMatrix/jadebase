/* 
 * bqt_launchargs.cpp
 * 
 * Implements bqt_launchargs.hpp & holds launch arguments state
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <iostream>

#include "bqt_launchargs.hpp"

#include "bqt_exception.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    enum launch_arg_flag
    {
        LF_INVALID,
        LF_UNRECOGNIZED,
        LF_DEVMODE,                                                             // --devmode, -d
        LF_LOGFILE                                                              // --logfile path, -l path
    };
    
    launch_arg_flag str2lf( std::string str )
    {
        if( str.length() < 1                                                    // Case for null string
            || str[ 0 ] != '-'                                                  // Case for flag with no dash
            || ( str[ 0 ] == '-' && str.length() < 2 )                          // Case for only a dash
            || ( str[ 1 ] == '-' && str.length() < 3 ) )                        // Case for only double-dash
            return LF_INVALID;
        
        if( str == LAUNCHARG_DEVMODE || str == LAUNCHFLG_DEVMODE )
            return LF_DEVMODE;
        if( str == LAUNCHARG_LOGFILE || str == LAUNCHFLG_LOGFILE )
            return LF_LOGFILE;
        
        return LF_UNRECOGNIZED;
    }
    
    // Engine options are immutable after parseLaunchArgs is called.
    bool        dev_mode;
    std::string log_file_name;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    #define INCMSG( msg )\
    {\
        i++;\
        if( i >= argc )\
            throw bqt::exception( msg );\
    }                       
    
    void parseLaunchArgs( int argc, char* argv[] )
    {
        dev_mode      = LAUNCHVAL_DEVMODE;
        log_file_name = LAUNCHVAL_LOGFILE;
        
        for( int i = 1; i < argc; i++ )
        {
            switch( str2lf( argv[ i ] ) )
            {
            case LF_INVALID:
                throw bqt::exception( "Invalid launch flag: '" + std::string( argv[ i ] ) + "'" );
            case LF_UNRECOGNIZED:
                throw bqt::exception( "Unrecognized launch flag: '" + std::string( argv[ i ] ) + "'" );
                
            case LF_DEVMODE:
                {
                    dev_mode = true;
                }
                break;
            case LF_LOGFILE:
                {
                    INCMSG( "Log file name missing" );
                    log_file_name = argv[ i ];
                    // TODO: redirect log messages to log
                }
                break;
                
            default:
                throw bqt::exception( "Something went wrong while parsing launch arguments!" );
            }
        }
    }
}


