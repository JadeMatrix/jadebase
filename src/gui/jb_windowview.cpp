/* 
 * jb_windowview.cpp
 * 
 * Implements jade::windowview class
 * 
 * Notice that none of these member implementations lock element_mutex - there's
 * no need to as they only reference this and parent_window, neither of which
 * will change for the lifetime of the object.
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_windowview.hpp"

#include "../utility/jb_exception.hpp"
#include "../utility/jb_launchargs.hpp"
#include "../utility/jb_log.hpp"
#include "../windowsys/jb_window.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    windowview::windowview( window* w ) : group( 0, 0, 0, 0 )
    {
        if( w == NULL )
            throw exception( "windowview::windowview(): NULL parent window" );
        
        parent_window = w;
        parent = NULL;
        
        // Window will set dimensions later
    }
    
    void windowview::setParentElement( gui_element* p )
    {
        if( getDevMode() )
            ff::write( jb_out,
                       "Warning: windowview::setParentElement(): Attempt to set parent element" );
    }
    
    void windowview::requestRedraw()
    {
        parent_window -> requestRedraw();
    }
    
    void windowview::associateDevice( jb_platform_idevid_t dev_id,
                                      std::list< gui_element* >& chain )
    {
        chain.push_front( this );
        
        parent_window -> associateDevice( dev_id, chain );
    }
    void windowview::associateDevice( jb_platform_idevid_t dev_id )
    {
        std::list< gui_element* > chain;                                        // Stack allocated OK - will be copied/swapped by window
        chain.push_front( this );
        
        parent_window -> associateDevice( dev_id, chain );
    }
    void windowview::deassociateDevice( jb_platform_idevid_t dev_id )
    {
        parent_window -> deassociateDevice( dev_id );
    }
}


