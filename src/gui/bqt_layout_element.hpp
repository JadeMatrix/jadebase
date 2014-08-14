#ifndef BQT_LAYOUT_ELEMENT_HPP
#define BQT_LAYOUT_ELEMENT_HPP

/* 
 * bqt_layout_element.hpp
 * 
 * Base class for GUI elements
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_layout.hpp"
#include "../bqt_windowevent.hpp"
#include "../bqt_mutex.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    class layout_element
    {
    protected:
        mutex element_mutex;
        layout& parent;
        
        unsigned int dimensions[ 2 ];                                           // Does not necessarily have to be used by child classes
        int position[ 2 ];
        
        bool event_fallthrough;                                                 // Allow events to fall through if not accepted; true by default, should remain
                                                                                // true for wrapper elements
    public:
        layout_element( layout& parent,
                        int x,
                        int y,
                        unsigned int w,
                        unsigned int h );
        virtual ~layout_element() {};
        
        void setRealDimensions( unsigned int w, unsigned int h );
        virtual std::pair< unsigned int, unsigned int > getRealDimensions();    // Used to arrange elements
        virtual std::pair< unsigned int, unsigned int > getVisualDimensions();  // Used to generate area for event capturing
        
        void setPosition( int x, int y );
        std::pair< int, int > getPosition();
        
        void setEventFallthrough( bool f );
        bool getEventFallthrough();
        
        virtual bool acceptEvent( window_event&e ) = 0;                         // If the event was accepted, returns a pointer to the layout_element that
                                                                                // accepted, else returns NULL.  If event_fallthrough is false should always
                                                                                // returns a pointer to a layout_element, even it it's this.
        
        virtual void draw() = 0;
    };
}

/******************************************************************************//******************************************************************************/

#endif


