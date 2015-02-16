#ifndef JADEBASE_MAIN_H
#define JADEBASE_MAIN_H

/* 
 * jb_main.h
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

bool jb_parseLaunchArgs( int argc, char* argv[] );                              // Returns true if execution should continue; a C wrapper for the C++ function,
                                                                                // this handles any exceptions from jade::parseLaunchArgs()

int jb_main();                                                                  // No longer need argc & argv as we parse these before calling jb_main()

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


