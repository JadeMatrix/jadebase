#ifndef BQT_MAIN_H
#define BQT_MAIN_H

/* 
 * bqt_main.h
 * 
 * Contains the main function for wrapping by platform-specific code
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



/******************************************************************************//******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

bool bqt_parseLaunchArgs( int argc, char* argv[] );                             // Returns true if execution should continue; a C wrapper for the C++ function,
                                                                                // this handles any exceptions from bqt::parseLaunchArgs()

int bqt_main( /* int argc, char* argv[] */ );                                   // No longer need argc & argv as we parse these before calling bqt_main()

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


