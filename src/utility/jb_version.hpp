#ifndef JADEBASE_VERSION_HPP
#define JADEBASE_VERSION_HPP

/* 
 * jb_version.hpp
 * 
 * Version information
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_log.hpp"

/******************************************************************************//******************************************************************************/

#define JADEBASE_VERSION_MAJOR   0
#define JADEBASE_VERSION_MINOR   1
#define JADEBASE_VERSION_PATCH   0

#define JADEBASE_VERSION_STRING_M( maj, min, rev )   "jadebase alpha " MACROTOSTR( min )  // Version to use until we're out of alpha/beta
// #define JADEBASE_VERSION_STRING_M( maj, min, rev )   "jadebase v" MACROTOSTR( JADEBASE_VERSION_MAJOR ) \
//                                                      "." MACROTOSTR( JADEBASE_VERSION_MINOR ) \
//                                                      "." MACROTOSTR( JADEBASE_VERSION_PATCH )

#define JADEBASE_VERSION_STRING JADEBASE_VERSION_STRING_M( JADEBASE_VERSION_MAJOR, JADEBASE_VERSION_MINOR, JADEBASE_VERSION_PATCH )

/******************************************************************************//******************************************************************************/

#endif


