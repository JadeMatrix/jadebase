/* 
 * bqt_gui_tabset.cpp
 * 
 * reorganizeTabs() current does not allow the captured tab to follow the cursor
 * except in steps.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_tabset.hpp"

#include <cmath>

#include "bqt_gui_resource.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_log.hpp"
#include "bqt_named_resources.hpp"
#include "../bqt_gl.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::mutex tabset_rsrc_mutex;
    bool got_resources = false;
    
    struct ctrl_set
    {
        bqt::gui_resource* up;
        bqt::gui_resource* over;
        bqt::gui_resource* down;
    };
    
    struct tab_set
    {
        bqt::gui_resource* left;
        bqt::gui_resource* center;
        bqt::gui_resource* right;
        
        ctrl_set safe;
        ctrl_set unsafe;
    };
    
    struct
    {
        tab_set active;
        tab_set inactive;
        
        bqt::gui_resource* fill;
    } tabset_set;
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    int tabset::getTabIndex( group* g )
    {
        for( int i = 0; i < tabs.size(); ++ i )
        {
            if( tabs[ i ].contents == g )
                return i;
        }
        
        throw exception( "tabset::getTabIndex(): No such tab" );
    }
    void tabset::reorganizeTabs()                                               // May do more advanced stuff later
    {
        if( tabs.size() )
        {
            total_tab_width = 0;
            
            for( int i = 0; i < tabs.size(); ++i )
            {
                if( !( capturing && i == current_tab ) )
                    tabs[ i ].position = total_tab_width;                       // Don't set the position of the tab we're currently dragging
                
                tabs[ i ].width = tabs[ i ].title -> getDimensions().first + TABSET_MIN_TAB_WIDTH;
                
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
        }
        
        parent.requestRedraw();
    }
    
    tabset::tabset( window& parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : gui_element( parent, x, y, w, h )
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
        {
            tabs[ i ].contents -> close();
            
            delete tabs[ i ].contents;
            delete tabs[ i ].title;
        }
    }
    
    void tabset::addTab( group* g, std::string t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        ff::write( bqt_out,
                   "Adding tab, setting contents dimensions to ",
                   dimensions[ 0 ],
                   " x ",
                   dimensions[ 1 ] - TABSET_BAR_HEIGHT,
                   "\n" );
        
        g -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
        g -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        
        tab_data new_data;
        
        new_data.contents = g;
        new_data.state = tab_data::CLOSE_SAFE;
        new_data.button_state = tab_data::UP;
        new_data.position = 0;
        new_data.width = TABSET_MIN_TAB_WIDTH;
        
        new_data.title = new text_rsrc( 11.0f, GUI_LABEL_FONT, t );
        new_data.title -> setMaxDimensions( TABSET_MAX_TITLE_WIDTH,
                                            -1, // TEXT_MAXHEIGHT_ONELINE,
                                            text_rsrc::END );
        
        tabs.push_back( new_data );
        
        if( current_tab < 0 )
        {
            current_tab = 0;
            tabs[ current_tab ].contents -> shown();
        }
        
        reorganizeTabs();                                                       // Calls parent.requestRedraw()
    }
    void tabset::removeTab( group* g )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        int i = 0;
        for( std::vector< tab_data >::iterator iter = tabs.begin();
             iter != tabs.end();
             ++iter )
        {
            if( iter -> contents == g )
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
                            tabs[ current_tab + 1 ].contents -> shown();        // ... select the next to the right as the new current (index remains the same)
                        else
                        {
                            --current_tab;                                      // ... otherwise select the one to the left
                            if( current_tab >= 0 )
                                tabs[ current_tab ].contents -> shown();
                            else
                                current_tab = -1;                               // ... unless there are no more tabs, then we have no current
                        }
                    }                                                           // Removing a tab to the right of current does not affect current
                }
                
                delete iter -> title;
                
                // We do not delete the contents or call its close()
                
                tabs.erase( iter );
                
                reorganizeTabs();                                               // Calls parent.requestRedraw()
                
                return;
            }
            
            ++i;
        }
        
        throw exception( "tabset::removeTab(): No such tab" );
    }
    
    void tabset::setTabTitle( group* g, std::string t )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        tabs[ getTabIndex( g ) ].title -> setString( t );
        
        parent.requestRedraw();
    }
    void tabset::setTabSafe( group* g, bool safe )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( safe )
            tabs[ getTabIndex( g ) ].state = tab_data::CLOSE_SAFE;
        else
            tabs[ getTabIndex( g ) ].state = tab_data::CLOSE_UNSAFE;
        
        parent.requestRedraw();
    }
    void tabset::makeTabCurrent( group* g )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        if( current_tab >= 0 )
            tabs[ current_tab ].contents -> hidden();
        
        current_tab = getTabIndex( g );
        g -> shown();
        
        parent.requestRedraw();
    }
    // void tabset::moveTabToLeft( group* g )
    // {
    //     scoped_lock< mutex > slock( element_mutex );
        
        
        
    //     parent.requestRedraw();
    // }
    // void tabset::moveTabToRight( group* g )
    // {
    //     scoped_lock< mutex > slock( element_mutex );
        
        
        
    //     parent.requestRedraw();
    // }
    
    void tabset::setRealPosition( int x, int y )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        position[ 0 ] = x;
        position[ 1 ] = y;
        
        for( int i = 0; i < tabs.size(); ++i )
        {
            tabs[ i ].contents -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
            tabs[ i ].contents -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        }
        
        parent.requestRedraw();
    }
    void tabset::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< mutex > slock( element_mutex );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        for( int i = 0; i < tabs.size(); ++i )
        {
            tabs[ i ].contents -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
            tabs[ i ].contents -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        }
        
        reorganizeTabs();                                                       // Calls parent.requestRedraw()
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
                        // if( !bqt_platform_idevid_t_equal( e.stroke.dev_id, captured_dev ) )
                        //     capturing = false;
                        // else
                        {
                            if( e.stroke.click & CLICK_PRIMARY )
                            {
                                if( current_tab < 0 || current_tab >= tabs.size() )
                                    throw exception( "tabset::acceptEvent(): Capturing without valid tab" );
                                
                                tabs[ current_tab ].position = capture_start[ 2 ] + e.stroke.position[ 0 ] - e.offset[ 0 ] - capture_start[ 0 ];
                                
                                if( tabs[ current_tab ].position < 0 )
                                    tabs[ current_tab ].position = 0;
                                
                                if( current_tab > 0
                                    && tabs[ current_tab ].position
                                       <= tabs[ current_tab - 1 ].position + tabs[ current_tab - 1 ].width / 2 )
                                {
                                    tab_data temp = tabs[ current_tab - 1 ];
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
                                        tab_data temp = tabs[ current_tab + 1 ];
                                        tabs[ current_tab + 1 ] = tabs[ current_tab ];
                                        tabs[ current_tab ] = temp;
                                        
                                        current_tab++;
                                    }
                                }
                                
                                reorganizeTabs();                                           // Calls parent.requestRedraw()
                                
                                return true;
                            }
                            else
                            {
                                capturing = false;
                                parent.deassociateDevice( e.stroke.dev_id );
                                reorganizeTabs();
                                return true;
                            }
                        }
                    }
                    
                    if( !capturing )
                    {
                        if( pointInsideRect( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                             e.stroke.position[ 1 ] - e.offset[ 1 ],
                                             position[ 0 ],
                                             position[ 1 ],
                                             dimensions[ 0 ],
                                             TABSET_BAR_HEIGHT ) )              // Stroke inside tab bar
                        {
                            for( int i = 0; i < tabs.size(); ++i )
                            {
                                if( e.stroke.position[ 0 ] - e.offset[ 0 ] >= position[ 0 ] + tabs[ i ].position + bar_scroll
                                    && e.stroke.position[ 0 ] - e.offset[ 0 ] < position[ 0 ] + tabs[ i ].position + bar_scroll + tabs[ i ].width
                                    && e.stroke.position[ 1 ] - e.offset[ 1 ] < position[ 1 ] + TABSET_TAB_HEIGHT )
                                {
                                    if( pointInsideCircle( e.stroke.position[ 0 ] - e.offset[ 0 ],
                                                           e.stroke.position[ 1 ] - e.offset[ 1 ],
                                                           position[ 0 ] + tabs[ i ].position + tabs[ i ].width - 13 + bar_scroll,
                                                           position[ 1 ] + 13,
                                                           7 ) )                // Current stroke in button
                                    {
                                        if( e.stroke.click & CLICK_PRIMARY )
                                        {
                                            tabs[ i ].button_state = tab_data::DOWN;
                                            parent.requestRedraw();
                                        }
                                        else
                                        {
                                            if( tabs[ i ].button_state == tab_data::DOWN )
                                            {                                   // Close tab
                                                tabs[ i ].contents -> close();
                                                removeTab( tabs[ i ].contents );// Calls other utilities for cleanup
                                            }
                                            else                                // Just a mouseover
                                            {
                                                tabs[ i ].button_state = tab_data::OVER;
                                                parent.requestRedraw();
                                            }
                                        }
                                        
                                        return true;
                                    }
                                    else
                                    {
                                        if( pointInsideCircle( e.stroke.prev_pos[ 0 ] - e.offset[ 0 ],
                                                               e.stroke.prev_pos[ 1 ] - e.offset[ 1 ],
                                                               position[ 0 ] + tabs[ i ].position + tabs[ i ].width - 13 + bar_scroll,
                                                               position[ 1 ] + 13,
                                                               7 ) )            // Previous stroke in button
                                        {
                                            tabs[ i ].button_state = tab_data::UP;
                                            parent.requestRedraw();
                                            return true;
                                        }
                                        
                                        if( e.stroke.click & CLICK_PRIMARY )    // Start tab dragging
                                        {
                                            current_tab = i;
                                            
                                            capturing = true;
                                            capture_start[ 0 ] = e.stroke.position[ 0 ] - e.offset[ 0 ];
                                            capture_start[ 1 ] = e.stroke.position[ 1 ] - e.offset[ 1 ];
                                            capture_start[ 2 ] = tabs[ i ].position;
                                            
                                            parent.associateDevice( e.stroke.dev_id, this, e.offset[ 0 ], e.offset[ 1 ] );
                                            
                                            parent.requestRedraw();
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
                    if( pointInsideRect( e.scroll.position[ 0 ] - e.offset[ 0 ],
                                         e.scroll.position[ 1 ] - e.offset[ 1 ],
                                         position[ 0 ],
                                         position[ 1 ],
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
                        
                        reorganizeTabs();                                       // Clamps bar_scroll & calls parent.requestRedraw()
                        
                        return true;
                    }
                }
                break;
            default:
                break;
        }
        
        if( current_tab >= 0 )                                                  // current_tab is a group so it will convert event position for us
            return tabs[ current_tab ].contents -> acceptEvent( e );
        else
            return false;                                                       // If there is no current tab, we just let the event fall through
    }
    
    void tabset::draw()
    {
        scoped_lock< mutex > slock_e( element_mutex );
        scoped_lock< mutex > slock_r( tabset_rsrc_mutex );
        
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
                
                tabset_set.fill -> draw();
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
                        
                        switch( tabs[ i ].state )
                        {
                        case tab_data::CLOSE_SAFE:
                            c_set = &( t_set -> safe );
                            break;
                        case tab_data::CLOSE_UNSAFE:
                            c_set = &( t_set -> unsafe );
                            break;
                        default:
                            throw exception( "tabset::draw(): Unknown state" );
                        }
                        
                        t_set -> left -> draw();
                        
                        glTranslatef( 6.0f, 0.0f, 0.0f );
                        
                        glPushMatrix();
                        {
                            glScalef( tabs[ i ].width - 12.0f, 1.0f, 1.0f );
                            
                            t_set -> center -> draw();
                        }
                        glPopMatrix();
                        
                        glTranslatef( tabs[ i ].width - 12.0f, 0.0f, 0.0f );
                        
                        t_set -> right -> draw();
                        
                        glPushMatrix();
                        {
                            glTranslatef( -17.0f, 2.0f, 0.0f );
                            
                            switch( tabs[ i ].button_state )
                            {
                            case tab_data::UP:
                                c_set -> up -> draw();
                                break;
                            case tab_data::OVER:
                                c_set -> over -> draw();
                                break;
                            case tab_data::DOWN:
                                c_set -> down -> draw();
                                break;
                            default:
                                throw exception( "tabset::draw(): Unknown button state" );
                            }
                        }
                        glPopMatrix();
                        
                        glPushMatrix();
                        {
                            glTranslatef( 9.0f - ( tabs[ i ].width - 6.0f ), 17.0f, 0.0f );
                            
                            tabs[ i ].title -> setColor( 0.8f, 0.8f, 0.8f, 1.0f );
                            tabs[ i ].title -> draw();
                        }
                        glPopMatrix();
                        
                        glTranslatef( 6.0f, 0.0f, 0.0f );
                    }
                }
            }
            glPopMatrix();
            
            if( current_tab >= 0 )
            {
                t_set = &tabset_set.active;
                
                switch( tabs[ current_tab ].state )
                {
                case tab_data::CLOSE_SAFE:
                    c_set = &( t_set -> safe );
                    break;
                case tab_data::CLOSE_UNSAFE:
                    c_set = &( t_set -> unsafe );
                    break;
                default:
                    throw exception( "tabset::draw(): Unknown state for current tab" );
                }
                
                glPushMatrix();
                {
                    glTranslatef( tabs[ current_tab ].position + bar_scroll, 0.0f, 0.0f );
                    
                    t_set -> left -> draw();
                    
                    glTranslatef( 6.0f, 0.0f, 0.0f );
                    
                    glPushMatrix();
                    {
                        glScalef( tabs[ current_tab ].width - 12.0f, 1.0f, 1.0f );
                        
                        t_set -> center -> draw();
                    }
                    glPopMatrix();
                    
                    glTranslatef( tabs[ current_tab ].width - 12.0f, 0.0f, 0.0f );
                    
                    t_set -> right -> draw();
                    
                    glPushMatrix();
                    {
                        glTranslatef( -17.0f, 2.0f, 0.0f );
                        
                        switch( tabs[ current_tab ].button_state )
                        {
                        case tab_data::UP:
                            c_set -> up -> draw();
                            break;
                        case tab_data::OVER:
                            c_set -> over -> draw();
                            break;
                        case tab_data::DOWN:
                            c_set -> down -> draw();
                            break;
                        default:
                            throw exception( "tabset::draw(): Unknown button state for current tab" );
                        }
                    }
                    glPopMatrix();
                    
                    glPushMatrix();
                    {
                        glTranslatef( 9.0f - ( tabs[ current_tab ].width - 6.0f ), 17.0f, 0.0f );
                        
                        tabs[ current_tab ].title -> setColor( 1.0f, 1.0f, 1.0f, 1.0f );
                        tabs[ current_tab ].title -> draw();
                    }
                    glPopMatrix();
                }
                glPopMatrix();
                
                clearDrawMasks();                                               // Clear tab bar mask before drawing contents
                
                tabs[ current_tab ].contents -> draw();
            }
            else
                clearDrawMasks();                                               // Just clear tab bar mask
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


