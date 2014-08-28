#ifndef BQT_GUI_ELEMENT_HPP
#define BQT_GUI_ELEMENT_HPP

/* 
 * bqt_gui_element.hpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <utility>

#include "../threading/bqt_mutex.hpp"
#include "../bqt_window.hpp"
#include "../bqt_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
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


