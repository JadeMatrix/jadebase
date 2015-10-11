#ifndef JADEBASE_ELEMENT_HPP
#define JADEBASE_ELEMENT_HPP

/* 
 * jb_element.hpp
 * 
 * Base class for gui elements
 * 
 */

// TODO: Change positions & dimensions for gui elements & resources to non-pixel values for non-pixel-based displays

/* INCLUDES *******************************************************************//******************************************************************************/

#include <list>
#include <utility>

#include "../threading/jb_mutex.hpp"
#include "../utility/jb_callback.hpp"
#include "../utility/jb_dpi.hpp"
#include "../utility/jb_platform.h"
#include "../windowsys/jb_windowevent.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    // jade::window predeclared in jb_dpi.hpp
    
    class gui_element
    {
    public:
        gui_element( dpi::points,                                               // Initial X position in points
                     dpi::points,                                               // Initial Y position in points
                     dpi::points,                                               // Initial (potentially final) width in points
                     dpi::points );                                             // Initial (potentially final) height in points
        virtual ~gui_element();
        
        gui_element* getParentElement();
        virtual void setParentElement( gui_element* );                          // 'Collection' elements need to override this to update their children
        
        virtual void setRealPosition( dpi::points, dpi::points );               // X, Y; virtual as derived classes may need to update contents
        std::pair< dpi::points, dpi::points > getRealPosition();
        virtual std::pair< dpi::points, dpi::points > getVisualPosition();
        
        virtual std::pair< dpi::points, dpi::points > getRealDimensions();      // Used to arrange elements
        virtual std::pair< dpi::points, dpi::points > getVisualDimensions();    // Used to generate area for event capturing
        
        // TODO: Put elements in charge of normalizing event offsets to their own position
        virtual bool acceptEvent( window_event& ) = 0;                          // If the event was accepted, returns true, else returns false.  If
                                                                                // event_fallthrough is false should always return true.
        
        // TODO: Investigate if this can be a ref instead
        virtual void draw( window* ) = 0;                                       // Perform this element's OpenGL draw callback; the drawing window is neccessary
                                                                                // to get information about the environment in which the element is rendered.
        
        // TODO: Optional passing of a gui_element* for rendering just that area
        virtual void requestRedraw();                                           // Send a redraw request up the element graph; meant for children to call on
                                                                                // their parent, but can be called directly on any element.  Most elements will
                                                                                // not need to supply a custom implementation.
        
    protected:
        mutex element_mutex;
        gui_element* parent;
        
        dpi::points position[   2 ];
        dpi::points dimensions[ 2 ];
        
        virtual void setRealDimensions( dpi::points, dpi::points );             // Width, height; virtual as not all elements have flexible dimensions
        
        virtual void   associateDevice( jb_platform_idevid_t,                   // ID of the device to associate
                                        std::list< gui_element* >& );           // Capturing element chain
                                                                                // Begins sending input events from the device directly to the element without
                                                                                // passing through the element tree, using the given event offsets.
        virtual void   associateDevice( jb_platform_idevid_t );                 // Overload for when capturing element is initial caller
        virtual void deassociateDevice( jb_platform_idevid_t );                 // Called when an association is no longer necessary; elements must deassociate
                                                                                // all associated devices before destruction.
        // TODO: this etc. IMPORTANT:
        // virtual void clearAssociatedDevices();                                  // Utility function, to be called by setParentElement() when the new parent !=
                                                                                // the old one, so that we release all associated devices.  No-op for elements
                                                                                // that don't need it.
    };
}

/******************************************************************************//******************************************************************************/

#endif


