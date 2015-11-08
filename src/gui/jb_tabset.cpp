/* 
 * jb_tabset.cpp
 * 
 * Implements jade::tabset class
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_tabset.hpp"

#include <cmath>

#include "jb_named_resources.hpp"
#include "jb_resource.hpp"
#include "../utility/jb_exception.hpp"
#include "../utility/jb_gl.hpp"
#include "../utility/jb_log.hpp"
#include "../utility/jb_settings.hpp"
#include "../windowsys/jb_window.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    jade::mutex tabset_rsrc_mutex;
    bool got_resources = false;
    
    struct ctrl_set
    {
        jade::gui_resource* up;
        jade::gui_resource* over;
        jade::gui_resource* down;
    };
    
    struct tab_set
    {
        jade::gui_resource* left;
        jade::gui_resource* center;
        jade::gui_resource* right;
        
        ctrl_set safe;
        ctrl_set unsafe;
    };
    
    struct
    {
        tab_set active;
        tab_set inactive;
        
        jade::gui_resource* fill;
    } tabset_set;
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    // TABSET::TAB /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    tabset::tab::tab( tabset* p,
                      std::string t,
                      std::shared_ptr< group >& c ) : contents( c )
    {
        if( !contents )
            throw exception( "tabset::tab::tab(): Contents empty shared_ptr" );
        
        parent = p;
        contents -> setParentElement( p );
        
        title = new text_rsrc( 11.0f, GUI_LABEL_FONT, t );
        title -> setMaxDimensions( TABSET_MAX_TITLE_WIDTH,
                                   -1, // TEXT_MAXHEIGHT_ONELINE,
                                   text_rsrc::END );
        title -> setString( t );
        
        safe = true;
    }
    tabset::tab::~tab()
    {
        contents -> setParentElement( NULL );                                   // To prevent more NULL-parent errors in associateDevice(), I guess
        delete title;
    }
    
    void tabset::tab::setTitle( std::string t )
    {
        scoped_lock< mutex > slock( tab_mutex );
        
        title -> setString( t );
        
        if( parent != NULL )
            if( parent -> parent != NULL )
                parent -> parent -> requestRedraw();
    }
    std::string tabset::tab::getTitle()
    {
        scoped_lock< mutex > slock( tab_mutex );
        
        return title -> getString();
    }
    
    void tabset::tab::setSafe( bool s )
    {
        scoped_lock< mutex > slock( tab_mutex );
        
        safe = s;
    }
    bool tabset::tab::getSafe()
    {
        scoped_lock< mutex > slock( tab_mutex );
        
        return safe;
    }
    
    void tabset::tab::setParentTabset( tabset* p )
    {
        scoped_lock< mutex > slock( tab_mutex );
        
        if( parent != p )
            contents -> clearDeviceAssociations();
        
        parent = p;
        
        contents -> setParentElement( p );
    }
    
    // TABSET //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    tabset::tabset( dpi::points x,
                    dpi::points y,
                    dpi::points w,
                    dpi::points h ) : gui_element( x, y, w, h )
    {
        current_tab = -1;
        total_tab_width = 0;
        bar_scroll = 0;
        
        capturing = false;
        
        scoped_lock< mutex > slock( tabset_rsrc_mutex );
        
        if( !got_resources )
        {
            tabset_set.active.left        = getNamedResource( tab_active_left );
            tabset_set.active.center      = getNamedResource( tab_active_center );
            tabset_set.active.right       = getNamedResource( tab_active_right );
            tabset_set.active.safe.up     = getNamedResource( tab_control_active_close_up );
            tabset_set.active.safe.over   = getNamedResource( tab_control_active_close_over );
            tabset_set.active.safe.down   = getNamedResource( tab_control_active_close_down );
            tabset_set.active.unsafe.up   = getNamedResource( tab_control_active_unsaved_up );
            tabset_set.active.unsafe.over = getNamedResource( tab_control_active_unsaved_over );
            tabset_set.active.unsafe.down = getNamedResource( tab_control_active_unsaved_down );
            
            tabset_set.inactive.left        = getNamedResource( tab_inactive_left );
            tabset_set.inactive.center      = getNamedResource( tab_inactive_center );
            tabset_set.inactive.right       = getNamedResource( tab_inactive_right );
            tabset_set.inactive.safe.up     = getNamedResource( tab_control_inactive_close_up );
            tabset_set.inactive.safe.over   = getNamedResource( tab_control_inactive_close_over );
            tabset_set.inactive.safe.down   = getNamedResource( tab_control_inactive_close_down );
            tabset_set.inactive.unsafe.up   = getNamedResource( tab_control_inactive_unsaved_up );
            tabset_set.inactive.unsafe.over = getNamedResource( tab_control_inactive_unsaved_over );
            tabset_set.inactive.unsafe.down = getNamedResource( tab_control_inactive_unsaved_down );
            
            tabset_set.fill = getNamedResource( tab_fill );
            
            got_resources = true;
        }
    }
    tabset::~tabset()
    {
        for( int i = 0; i < tabs.size(); ++i )
            tabs[ i ].data -> contents -> closed();
        // Tabs destroyed on return, which decrements their contents' shared_ptr
    }
    
    void tabset::addTab( std::shared_ptr< tab >& t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        tab_state new_state( t );
        
        new_state.button_state = tab_state::UP;
        new_state.position = 0;
        new_state.width = TABSET_MIN_TAB_WIDTH;
        
        new_state.data -> setParentTabset( this );
        
        new_state.data -> contents -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
        new_state.data -> contents -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        new_state.data -> contents -> setParentElement( this );
        
        tabs.push_back( new_state );
        
        if( current_tab < 0 )
        {
            current_tab = 0;
            tabs[ current_tab ].data -> contents -> shown();
        }
        
        reorganizeTabs();                                                       // May call parent -> requestRedraw()
    }
    void tabset::removeTab( std::shared_ptr< tab >& t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        int i = 0;
        for( std::vector< tab_state >::iterator iter = tabs.begin();
             iter != tabs.end();
             ++iter )
        {
            if( iter -> data == t )
            {
                if( i < current_tab )                                           // If removed tab is to the left of current
                {
                    --current_tab;                                              // ... current's new position is one to the left
                }
                else
                {
                    if( i == current_tab )                                      // If we are removing the current tab
                    {
                        if( current_tab < tabs.size() - 1 )                     // If current is not at the far right
                            tabs[ current_tab + 1 ].data -> contents -> shown();// ... select the next to the right as the new current (index remains the same)
                        else
                        {
                            --current_tab;                                      // ... otherwise select the one to the left
                            if( current_tab >= 0 )
                                tabs[ current_tab ].data -> contents -> shown();
                            else
                                current_tab = -1;                               // ... unless there are no more tabs, then we have no current
                        }
                    }                                                           // Removing a tab to the right of current does not affect current
                }
                
                // We do not delete the tab or call its content's closed()
                
                iter -> data -> setParentTabset( NULL );
                tabs.erase( iter );
                
                reorganizeTabs();                                               // May call parent -> requestRedraw()
                
                return;
            }
            
            ++i;
        }
        
        throw exception( "tabset::removeTab(): No such tab" );
    }
    
    void tabset::makeTabCurrent( std::shared_ptr< tab >& t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        int new_index = current_tab = getTabIndex( t );
        
        if( new_index != current_tab )
        {
            if( current_tab >= 0 )
                tabs[ current_tab ].data -> contents -> hidden();
            
            current_tab = new_index;
            t -> contents -> shown();
            
            if( parent != NULL )
                parent -> requestRedraw();
        }
    }
    void tabset::moveTabLeft( std::shared_ptr< tab >& t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        int index = getTabIndex( t );
        
        if( index != 0                                                          // If it's 0 we can't move it to the left (no need to redraw either)
            && !( capturing && index == current_tab ) )                         // Don't try shifting the current tab while it is being moved manually, either
        {
            for( int i = 0; i < tabs.size(); ++i )
            {
                if( i == index - 1                                              // Just to the left of the tab we're moving
                    && !( capturing && i == current_tab ) )                     // ... but again not if it's the current tab and being moved manually
                {
                    int pos_temp = tabs[ i ].position;
                    tabs[ i ].position = tabs[ index ].position;
                    tabs[ index ].position = pos_temp;
                    
                    if( parent != NULL )
                        parent -> requestRedraw();                              // No need to call reorganizeTabs(), but we do want to try to redraw
                }
            }
        }
    }
    void tabset::moveTabRight( std::shared_ptr< tab >& t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        int index = getTabIndex( t );
        
        if( index != tabs.size() - 1                                            // If it's already all the way we can't move it to the right
            && !( capturing && index == current_tab ) )                         // Don't try shifting the current tab while it is being moved manually, either
        {
            for( int i = 0; i < tabs.size(); ++i )
            {
                if( i == index + 1                                              // Just to the left of the tab we're moving
                    && !( capturing && i == current_tab ) )                     // ... but again not if it's the current tab and being moved manually
                {
                    int pos_temp = tabs[ i ].position;
                    tabs[ i ].position = tabs[ index ].position;
                    tabs[ index ].position = pos_temp;
                    
                    if( parent != NULL )
                        parent -> requestRedraw();                              // No need to call reorganizeTabs(), but we do want to try to redraw
                }
            }
        }
    }
    
    void tabset::setRealPosition( dpi::points x, dpi::points y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        for( int i = 0; i < tabs.size(); ++i )
            tabs[ i ].data -> contents -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
        
        if( parent != NULL )
            parent -> requestRedraw();
    }
    void tabset::setRealDimensions( dpi::points w, dpi::points h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        for( int i = 0; i < tabs.size(); ++i )
            tabs[ i ].data -> contents -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        
        reorganizeTabs();                                                       // May call parent -> requestRedraw()
    }
    
    bool tabset::acceptEvent( window_event& e )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        switch( e.type )
        {
            case STROKE:
                {
                    if( capturing )
                    {
                        if( e.stroke.dev_id != captured_dev )                   // Ignore other devices wile capturing another
                            break;
                        
                        if( e.stroke.click & CLICK_PRIMARY )
                        {
                            if( current_tab < 0 || current_tab >= tabs.size() )
                                throw exception( "tabset::acceptEvent(): Capturing without valid tab" );
                            
                            tabs[ current_tab ].position = capture_start[ 2 ] + e.stroke.position[ 0 ] - capture_start[ 0 ];
                            
                            if( tabs[ current_tab ].position < 0 )
                                tabs[ current_tab ].position = 0;
                            
                            if( current_tab > 0
                                && tabs[ current_tab ].position
                                   <= tabs[ current_tab - 1 ].position + tabs[ current_tab - 1 ].width / 2 )
                            {
                                tab_state temp = tabs[ current_tab - 1 ];
                                tabs[ current_tab - 1 ] = tabs[ current_tab ];
                                tabs[ current_tab ] = temp;
                                
                                current_tab--;
                            }
                            else
                            {
                                if( current_tab < tabs.size() - 1
                                    && tabs[ current_tab ].position + tabs[ current_tab ].width
                                       >= tabs[ current_tab + 1 ].position + tabs[ current_tab + 1 ].width / 2 )
                                {
                                    tab_state temp = tabs[ current_tab + 1 ];
                                    tabs[ current_tab + 1 ] = tabs[ current_tab ];
                                    tabs[ current_tab ] = temp;
                                    
                                    current_tab++;
                                }
                            }
                            
                            reorganizeTabs();                                   // May call parent -> requestRedraw()
                            
                            return true;
                        }
                        else
                        {
                            capturing = false;
                            deassociateDevice( e.stroke.dev_id );
                            reorganizeTabs();
                            
                            return pointInsideRect( e.stroke.position[ 0 ],
                                                    e.stroke.position[ 1 ],
                                                    0,
                                                    0,
                                                    dimensions[ 0 ],
                                                    dimensions[ 1 ] );          // If inside tabset, return true else return false
                        }
                    }
                    else
                    {
                        if( pointInsideRect( e.stroke.position[ 0 ],
                                             e.stroke.position[ 1 ],
                                             0,
                                             0,
                                             dimensions[ 0 ],
                                             TABSET_BAR_HEIGHT ) )              // Stroke inside tab bar
                        {
                            for( int i = 0; i < tabs.size(); ++i )
                            {
                                if( e.stroke.position[ 0 ] >= position[ 0 ] + tabs[ i ].position + bar_scroll
                                    && e.stroke.position[ 0 ] < position[ 0 ] + tabs[ i ].position + bar_scroll + tabs[ i ].width
                                    && e.stroke.position[ 1 ] < position[ 1 ] + TABSET_TAB_HEIGHT )
                                {
                                    if( pointInsideCircle( e.stroke.position[ 0 ],
                                                           e.stroke.position[ 1 ],
                                                           tabs[ i ].position + tabs[ i ].width - 13 + bar_scroll,
                                                           13,
                                                           7 ) )                // Current stroke in button
                                    {
                                        if( e.stroke.click & CLICK_PRIMARY )
                                        {
                                            tabs[ i ].button_state = tab_state::DOWN;
                                            if( parent != NULL )
                                                parent -> requestRedraw();
                                        }
                                        else
                                        {
                                            if( tabs[ i ].button_state == tab_state::DOWN )
                                            {                                   // Close tab
                                                tabs[ i ].data -> contents -> closed();
                                                removeTab( tabs[ i ].data );    // Calls other utilities for cleanup
                                            }
                                            else                                // Just a mouseover
                                                tabs[ i ].button_state = tab_state::OVER;
                                            
                                            if( parent != NULL )
                                                parent -> requestRedraw();
                                        }
                                        
                                        return true;
                                    }
                                    else
                                    {
                                        if( pointInsideCircle( e.stroke.prev_pos[ 0 ],
                                                               e.stroke.prev_pos[ 1 ],
                                                               tabs[ i ].position + tabs[ i ].width - 13 + bar_scroll,
                                                               13,
                                                               7 ) )            // Previous stroke in button
                                        {
                                            tabs[ i ].button_state = tab_state::UP;
                                            if( parent != NULL )
                                                parent -> requestRedraw();
                                            return true;
                                        }
                                        
                                        if( e.stroke.click & CLICK_PRIMARY )    // Start tab dragging
                                        {
                                            current_tab = i;
                                            
                                            capturing = true;
                                            capture_start[ 0 ] = e.stroke.position[ 0 ];
                                            capture_start[ 1 ] = e.stroke.position[ 1 ];
                                            capture_start[ 2 ] = tabs[ i ].position;
                                            
                                            associateDevice( e.stroke.dev_id );
                                            captured_dev = e.stroke.dev_id;
                                            
                                            if( parent != NULL )
                                                parent -> requestRedraw();
                                        }
                                        
                                        return true;                            // Either way we used the event
                                    }
                                }
                                
                                // Will return in some way if true
                            }
                            
                            return true;                                        // Stroke in bar but not on a tab
                        }
                        
                        // Let event fall through to curren tab's contents
                    }
                }
                break;
            case SCROLL:
                {
                    if( pointInsideRect( e.scroll.position[ 0 ],
                                         e.scroll.position[ 1 ],
                                         0,
                                         0,
                                         dimensions[ 0 ],
                                         TABSET_BAR_HEIGHT ) )
                    {
                        if( e.scroll.amount[ 0 ] != 0.0f )                      // If there is any horizontal scroll, use that
                        {
                            if( capturing )
                                capture_start[ 0 ] += e.scroll.amount[ 0 ];     // Readjust capture start X so tab follows cursor while scrolling
                            
                            bar_scroll += e.scroll.amount[ 0 ];
                        }
                        else                                                    // Otherwise use only vertical scroll
                        {
                            if( capturing )
                                capture_start[ 0 ] += e.scroll.amount[ 1 ];     // Readjust capture start X so tab follows cursor while scrolling
                            
                            bar_scroll += e.scroll.amount[ 1 ];
                        }
                        
                        reorganizeTabs();                                       // Clamps bar_scroll & May call parent -> requestRedraw()
                        
                        return true;
                    }
                }
                break;
            default:
                break;
        }
        
        if( current_tab >= 0 )
        {
            window_event e_copy = e;
            
            std::pair< dpi::points, dpi::points > contents_offset = tabs[ current_tab ].data -> contents -> getEventOffset();
            
            switch( e_copy.type )
            {
            case STROKE:
                e_copy.stroke.position[ 0 ] -= contents_offset.first;
                e_copy.stroke.position[ 1 ] -= contents_offset.second;
                e_copy.stroke.prev_pos[ 0 ] -= contents_offset.first;
                e_copy.stroke.prev_pos[ 1 ] -= contents_offset.second;
                break;
            case DROP:
                e_copy.drop.position[ 0 ] -= contents_offset.first;
                e_copy.drop.position[ 1 ] -= contents_offset.second;
                break;
            case KEYCOMMAND:
            case COMMAND:
            case TEXT:
                break;
            case PINCH:
                e_copy.pinch.position[ 0 ] -= contents_offset.first;
                e_copy.pinch.position[ 1 ] -= contents_offset.second;
                break;
            case SCROLL:
                e_copy.scroll.position[ 0 ] -= contents_offset.first;
                e_copy.scroll.position[ 1 ] -= contents_offset.second;
                break;
            default:
                throw exception( "tabset::acceptEvent(): Unknown event type" );
                break;
            }
            
            return tabs[ current_tab ].data -> contents -> acceptEvent( e_copy );
        }
        else
            return false;                                                       // If there is no current tab, we just let the event fall through
    }
    
    void tabset::draw( window* w )
    {
        scoped_lock< mutex > slock( element_mutex );
        tabset_rsrc_mutex.lock();                                               // We want to potentially unlock this early
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            addDrawMask( 0, 0, dimensions[ 0 ], TABSET_BAR_HEIGHT );
            
            glBegin( GL_QUADS );
            {
                glColor4f( 0.1f, 0.1f, 0.1f, 1.0f );
                
                glVertex2f( 0.0f, 0.0f );
                glVertex2f( 0.0f, TABSET_BAR_HEIGHT );
                glVertex2f( dimensions[ 0 ], TABSET_BAR_HEIGHT );
                glVertex2f( dimensions[ 0 ], 0.0f );
                
                glColor4f( 1.0, 1.0f, 1.0f, 1.0f );
            }
            glEnd();
            
            glPushMatrix();
            {
                glScalef( dimensions[ 0 ], 1.0f, 1.0f );
                
                tabset_set.fill -> draw( w );
            }
            glPopMatrix();
            
            tab_set* t_set;
            ctrl_set* c_set;
            
            glPushMatrix();
            {
                glTranslatef( bar_scroll, 0.0f, 0.0f );
                
                for( int i = 0; i < tabs.size(); ++i )
                {
                    if( i == current_tab )
                        glTranslatef( tabs[ i ].width, 0.0f, 0.0f );
                    else
                    {
                        t_set = &tabset_set.inactive;
                        if( tabs[ i ].data -> safe )
                            c_set = &( t_set -> safe );
                        else
                            c_set = &( t_set -> unsafe );
                        
                        t_set -> left -> draw( w );
                        
                        glTranslatef( 6.0f, 0.0f, 0.0f );
                        
                        glPushMatrix();
                        {
                            glScalef( tabs[ i ].width - 12.0f, 1.0f, 1.0f );
                            
                            t_set -> center -> draw( w );
                        }
                        glPopMatrix();
                        
                        glTranslatef( tabs[ i ].width - 12.0f, 0.0f, 0.0f );
                        
                        t_set -> right -> draw( w );
                        
                        glPushMatrix();
                        {
                            glTranslatef( -17.0f, 2.0f, 0.0f );
                            
                            switch( tabs[ i ].button_state )
                            {
                            case tab_state::UP:
                                c_set -> up -> draw( w );
                                break;
                            case tab_state::OVER:
                                c_set -> over -> draw( w );
                                break;
                            case tab_state::DOWN:
                                c_set -> down -> draw( w );
                                break;
                            default:
                                throw exception( "tabset::draw(): Unknown button state" );
                            }
                        }
                        glPopMatrix();
                        
                        glPushMatrix();
                        {
                            glTranslatef( 9.0f - ( tabs[ i ].width - 6.0f ), 17.0f, 0.0f );
                            
                            tabs[ i ].data -> title -> setColor( 0.8f, 0.8f, 0.8f, 1.0f );
                            tabs[ i ].data -> title -> draw( w );
                        }
                        glPopMatrix();
                        
                        glTranslatef( 6.0f, 0.0f, 0.0f );
                    }
                }
            }
            glPopMatrix();
            
            if( current_tab >= 0 )                                              // Draw current tab
            {
                t_set = &tabset_set.active;
                
                if( tabs[ current_tab ].data -> safe )
                    c_set = &( t_set -> safe );
                else
                    c_set = &( t_set -> unsafe );
                
                glPushMatrix();
                {
                    glTranslatef( tabs[ current_tab ].position + bar_scroll, 0.0f, 0.0f );
                    
                    t_set -> left -> draw( w );
                    
                    glTranslatef( 6.0f, 0.0f, 0.0f );
                    
                    glPushMatrix();
                    {
                        glScalef( tabs[ current_tab ].width - 12.0f, 1.0f, 1.0f );
                        
                        t_set -> center -> draw( w );
                    }
                    glPopMatrix();
                    
                    glTranslatef( tabs[ current_tab ].width - 12.0f, 0.0f, 0.0f );
                    
                    t_set -> right -> draw( w );
                    
                    glPushMatrix();
                    {
                        glTranslatef( -17.0f, 2.0f, 0.0f );
                        
                        switch( tabs[ current_tab ].button_state )
                        {
                        case tab_state::UP:
                            c_set -> up -> draw( w );
                            break;
                        case tab_state::OVER:
                            c_set -> over -> draw( w );
                            break;
                        case tab_state::DOWN:
                            c_set -> down -> draw( w );
                            break;
                        default:
                            throw exception( "tabset::draw(): Unknown button state for current tab" );
                        }
                    }
                    glPopMatrix();
                    
                    glPushMatrix();
                    {
                        glTranslatef( 9.0f - ( tabs[ current_tab ].width - 6.0f ), 17.0f, 0.0f );
                        
                        tabs[ current_tab ].data -> title -> setColor( 1.0f, 1.0f, 1.0f, 1.0f );
                        tabs[ current_tab ].data -> title -> draw( w );
                    }
                    glPopMatrix();
                }
                glPopMatrix();
                
                clearDrawMasks();                                               // Clear tab bar mask before drawing contents
                
                tabset_rsrc_mutex.unlock();                                     // Unlock now as we don't know how long the contents' draw will take
                
                tabs[ current_tab ].data -> contents -> draw( w );
            }
            else
            {
                clearDrawMasks();                                               // Just clear tab bar mask
                
                tabset_rsrc_mutex.unlock();
            }
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
    
    void tabset::clearDeviceAssociations()                                      // clearDeviceAssociations() is not required to be thread-safe
    {
        if( capturing )
            deassociateDevice( captured_dev );
    }
    
    int tabset::getTabIndex( std::shared_ptr< tab > t )
    {
        for( int i = 0; i < tabs.size(); ++ i )
        {
            if( tabs[ i ].data == t )
                return i;
        }
        
        throw exception( "tabset::getTabIndex(): No such tab" );
    }
    void tabset::reorganizeTabs()
    {
        if( tabs.size() )
        {
            total_tab_width = 0;
            
            for( int i = 0; i < tabs.size(); ++i )
            {
                if( !( capturing && i == current_tab ) )
                    tabs[ i ].position = total_tab_width;                       // Don't set the position of the tab we're currently dragging
                
                tabs[ i ].width = tabs[ i ].data -> title -> getDimensions().first + TABSET_MIN_TAB_WIDTH;
                
                if( tabs[ i ].width < TABSET_MIN_TAB_WIDTH )
                    tabs[ i ].width = TABSET_MIN_TAB_WIDTH;
                
                total_tab_width += tabs[ i ].width;
            }
            
            if( total_tab_width < dimensions[ 0 ]
                || bar_scroll > 0 )
            {
                bar_scroll = 0;
            }
            else
            {
                if( total_tab_width + bar_scroll < dimensions[ 0 ] )
                    bar_scroll = dimensions[ 0 ] - total_tab_width;
            }
            
            if( parent != NULL )
                parent -> requestRedraw();
        }
    }
}


