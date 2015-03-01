#ifndef JADEBASE_ELEMENT_HPP
#define JADEBASE_ELEMENT_HPP

/* 
 * jb_element.hpp
 * 
 * Base class for gui elements
 * 
 * Note on elements using window* instead of window&:
 * As windows share GL contexts, there is no need for all elements to be locked
 * to a single window.  In addition, we want to allow the API to be able to
 * move elements from one window to another easily.  A parent window can be
 * NULL; changes are applied but no redraw is requested from the (nonexistent)
 * window.  Exceptions may be thrown in the few cases where calling the member
 * function requires knowledge of the parent.  As elements track their own
 * positions, positions can be changed without a window (or other kind of
 * parent).
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include <utility>

#include "../threading/jb_mutex.hpp"
#include "../windowsys/jb_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    class window;                                                               // Predec because window needs shared_ptr< group >
    
    class gui_callback
    {
    public:
        virtual ~gui_callback() {};
        virtual void call() = 0;
    };
    
    class gui_element
    {
    protected:
        mutex element_mutex;
        window* parent;
        
        int position[ 2 ];
        unsigned int dimensions[ 2 ];
        
        virtual void setRealDimensions( unsigned int w, unsigned int h );       // Not all elements have flexible dimensions
    public:
        gui_element( window* parent,
                     int x,
                     int y,
                     unsigned int w,
                     unsigned int h );
        virtual ~gui_element();
        
        window* getParentWindow();
        virtual void setParentWindow( window* );                                // 'Collective' elements need to override this for their children
        
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


