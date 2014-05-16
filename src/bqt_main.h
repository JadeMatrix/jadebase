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

bool bqt_parseLaunchArgs( int argc, char* argv[] );                             // Wrapping this in C is easier than trying to parse once we're in NS

int bqt_main( /* int argc, char* argv[] */ );                                   // No longer need argc & argv as we parse these before calling bqt_main()

#ifdef __cplusplus
}
#endif

/******************************************************************************//******************************************************************************/

#endif


