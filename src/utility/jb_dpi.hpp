#ifndef JADEBASE_DPI_HPP
#define JADEBASE_DPI_HPP

/* 
 * jb_dpi.hpp
 * 
 * Types & utilities for point-pixel abstraction
 *
 * We assume square screen pixels for now; support for rectangular pixels may
 * come in the future, but jadebase is intended to target modern systems.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/



/******************************************************************************//******************************************************************************/

namespace jade
{
    class window;                                                               // Predeclaration for conversion functions
    
    namespace dpi
    {
        typedef float points;                                                   // Points may be frational but should be treated as integral
        typedef int   pixels;                                                   // Pixels are always whole, integral values
        typedef float percent;                                                  // Percents are always relative to something and have no innate value
    }
    
    dpi::pixels pt2px( window&, dpi::points );                                  // Convert a point to a pixel distance based on the given window's scaling
    dpi::points px2pt( window&, dpi::pixels );                                  // Convert a pixel to a point distance based on the given window's scaling
    
    const dpi::pixels STANDARD_DPI = 75;
    const dpi::pixels HIRES_DPI    = 150;
}

/******************************************************************************//******************************************************************************/

#endif


