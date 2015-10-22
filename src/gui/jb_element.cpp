/* 
 * jb_element.cpp
 * 
 * Implements jade::gui_element class
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_element.hpp"

#include "../windowsys/jb_window.hpp"

// DEBUG:
#include "../utility/jb_log.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
    gui_element::gui_element( dpi::points x, dpi::points y, dpi::points w, dpi::points h )
    {
        parent = NULL;
        setRealPosition( x, y );
        setRealDimensions( w, h );
        
        // DEBUG:
        ff::write( jb_out,
                   ">>> jade::gui_element at 0x",
                   ff::to_X( ( unsigned long )this, PTR_HEX_WIDTH, PTR_HEX_WIDTH ),
                   " constructed\n" );
    }
    gui_element::~gui_element()
    {
        clearDeviceAssociations();
        
        // DEBUG:
        ff::write( jb_out,
                   ">>> jade::gui_element at 0x",
                   ff::to_X( ( unsigned long )this, PTR_HEX_WIDTH, PTR_HEX_WIDTH ),
                   " destroyed\n" );
    }
    
    gui_element* gui_element::getParentElement()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return parent;
    }
    void gui_element::setParentElement( gui_element* p )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( parent != p )
            clearDeviceAssociations();
        
        parent = p;
        
        if( parent != NULL )
            parent -> requestRedraw();                                          // Babby's first redraw request
    }
    
    void gui_element::setRealPosition( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    std::pair< dpi::points, dpi::points > gui_element::getRealPosition()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::points, dpi::points >( position[ 0 ], position[ 1 ] );
    }
    std::pair< dpi::points, dpi::points > gui_element::getVisualPosition()
    {
        return getRealPosition();
    }
    
    std::pair< dpi::points, dpi::points > gui_element::getRealDimensions()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        return std::pair< dpi::points, dpi::points >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< dpi::points, dpi::points > gui_element::getVisualDimensions()
    {
        return getRealDimensions();
    }
    
    void gui_element::requestRedraw()
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    void gui_element::clearDeviceAssociations()
    {
        // No-op for elements that don't need it
    }
    
    void gui_element::setRealDimensions( dpi::points w, dpi::points h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    
    void gui_element::associateDevice( jb_platform_idevid_t dev_id,
                                       std::list< gui_element* >& chain )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        chain.push_front( this );
        
        if( parent != NULL )
            parent -> associateDevice( dev_id, chain );
        else
            throw exception( "element::associateDevice(): NULL parent" );
    }
    void gui_element::associateDevice( jb_platform_idevid_t dev_id )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        std::list< gui_element* > chain;                                        // Stack allocated OK - will be copied/swapped by window
        chain.push_front( this );
        
        if( parent != NULL )
            parent -> associateDevice( dev_id, chain );
        else
            throw exception( "element::associateDevice() nochain: NULL parent" );
    }
    void gui_element::deassociateDevice( jb_platform_idevid_t dev_id )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( parent != NULL )
            parent -> deassociateDevice( dev_id );
        else
            throw exception( "element::deassociateDevice(): NULL parent" );
    }
}


