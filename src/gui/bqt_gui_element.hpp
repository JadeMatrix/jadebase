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

#include "../threading/bqt_rwlock.hpp"
#include "../bqt_window.hpp"
#include "../bqt_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class gui_element
    {
    protected:
        rwlock element_lock;
        window& parent;
        
        int position[ 2 ];
        unsigned int dimensions[ 2 ];
        
        bool event_fallthrough;                                                 // Allow events to fall through if not accepted; true by default, should remain
                                                                                // true for wrapper elements
    public:
        gui_element( window& parent,
                     int x,
                     int y,
                     unsigned int w,
                     unsigned int h );
        virtual ~gui_element();
        
        void setPosition( int x, int y );
        std::pair< int, int > getPosition();
        
        void setRealDimensions( unsigned int w, unsigned int h );
        virtual std::pair< unsigned int, unsigned int > getRealDimensions();    // Used to arrange elements
        virtual std::pair< unsigned int, unsigned int > getVisualDimensions();  // Used to generate area for event capturing
        
        virtual void setEventFallthrough( bool f );                             // Children can override this so it does nothing or does something extra
        bool getEventFallthrough();
        
        virtual bool acceptEvent( window_event& e ) = 0;                        // If the event was accepted, returns true, else returns false.  If
                                                                                // event_fallthrough is false should always return true.
        
        virtual void draw() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


