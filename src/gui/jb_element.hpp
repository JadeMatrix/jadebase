#ifndef JADEBASE_ELEMENT_HPP
#define JADEBASE_ELEMENT_HPP

/* 
 * jb_element.hpp
 * 
 * Base class for gui elements
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <utility>

#include "../threading/jb_mutex.hpp"
#include "../jb_window.hpp"
#include "../jb_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class gui_element
    {
    protected:
        mutex element_mutex;
        window& parent;
        
        int position[ 2 ];
        unsigned int dimensions[ 2 ];
        
        virtual void setRealDimensions( unsigned int w, unsigned int h );       // Not all elements have flexible dimensions
    public:
        gui_element( window& parent,
                     int x,
                     int y,
                     unsigned int w,
                     unsigned int h );
        virtual ~gui_element();
        
        virtual void setRealPosition( int x, int y );                           // Virtual as derived classes may need to update contents
        std::pair< int, int > getRealPosition();
        virtual std::pair< int, int > getVisualPosition();
        
        virtual std::pair< unsigned int, unsigned int > getRealDimensions();    // Used to arrange elements
        virtual std::pair< unsigned int, unsigned int > getVisualDimensions();  // Used to generate area for event capturing
        
        virtual bool acceptEvent( window_event& e ) = 0;                        // If the event was accepted, returns true, else returns false.  If
                                                                                // event_fallthrough is false should always return true.
        
        virtual void draw() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


