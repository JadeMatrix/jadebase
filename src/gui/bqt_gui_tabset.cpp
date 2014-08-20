/* 
 * bqt_gui_tabset.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_gui_tabset.hpp"

#include "bqt_gui_resource.hpp"
#include "../bqt_exception.hpp"
#include "../bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
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
    
    struct tabset_set
    {
        tab_set active;
        tab_set inactive;
        
        bqt::gui_resource* fill;
        
        int count;
    };
    
    bqt::rwlock tabset_rsrc_lock;
    std::map< bqt::window*, tabset_set > tabset_sets;
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
            tabs[ 0 ].position = 0;
            
            if( tabs[ 0 ].width < TABSET_MIN_TAB_WIDTH )
                tabs[ 0 ].width = TABSET_MIN_TAB_WIDTH;
            // TODO: Recalculate width of [ 0 ] based on title
            
            for( int i = 1; i < tabs.size(); ++i )
            {
                tabs[ i ].position = tabs[ i - 1 ].position + tabs[ i - 1 ].width;
                
                if( tabs[ i ].width < TABSET_MIN_TAB_WIDTH )
                    tabs[ i ].width = TABSET_MIN_TAB_WIDTH;
                // TODO: Recalculate width of [ i ] based on title
            }
        }
        
        parent.requestRedraw();
    }
    
    tabset::tabset( window& parent,
                    int x,
                    int y,
                    unsigned int w,
                    unsigned int h ) : gui_element( parent, x, y, w, h + TABSET_BAR_HEIGHT )
    {
        current_tab = -1;
        
        bar_scroll = 0;
        
        capturing = false;
        
        scoped_lock< rwlock > slock( tabset_rsrc_lock, RW_WRITE );
        
        if( !tabset_sets.count( &parent ) )
        {
            tabset_set& window_set( tabset_sets[ &parent ] );
            
            window_set.active.left        = parent.getNamedResource( tab_active_left );
            window_set.active.center      = parent.getNamedResource( tab_active_center );
            window_set.active.right       = parent.getNamedResource( tab_active_right );
            window_set.active.safe.up     = parent.getNamedResource( tab_control_active_close_up );
            window_set.active.safe.over   = parent.getNamedResource( tab_control_active_close_over );
            window_set.active.safe.down   = parent.getNamedResource( tab_control_active_close_down );
            window_set.active.unsafe.up   = parent.getNamedResource( tab_control_active_unsaved_up );
            window_set.active.unsafe.over = parent.getNamedResource( tab_control_active_unsaved_over );
            window_set.active.unsafe.down = parent.getNamedResource( tab_control_active_unsaved_down );
            
            window_set.inactive.left        = parent.getNamedResource( tab_inactive_left );
            window_set.inactive.center      = parent.getNamedResource( tab_inactive_center );
            window_set.inactive.right       = parent.getNamedResource( tab_inactive_right );
            window_set.inactive.safe.up     = parent.getNamedResource( tab_control_inactive_close_up );
            window_set.inactive.safe.over   = parent.getNamedResource( tab_control_inactive_close_over );
            window_set.inactive.safe.down   = parent.getNamedResource( tab_control_inactive_close_down );
            window_set.inactive.unsafe.up   = parent.getNamedResource( tab_control_inactive_unsaved_up );
            window_set.inactive.unsafe.over = parent.getNamedResource( tab_control_inactive_unsaved_over );
            window_set.inactive.unsafe.down = parent.getNamedResource( tab_control_inactive_unsaved_down );
            
            window_set.fill = parent.getNamedResource( tab_fill );
            
            window_set.count = 1;
        }
        else
            tabset_sets[ &parent ].count++;
    }
    tabset::~tabset()
    {
        scoped_lock< rwlock > slock( tabset_rsrc_lock, RW_WRITE );
        
        for( int i = 0; i < tabs.size(); ++i )
            delete tabs[ i ].contents;
        
        tabset_sets[ &parent ].count--;
        
        if( tabset_sets[ &parent ].count < 1 )
            tabset_sets.erase( &parent );
    }
    
    void tabset::setRealDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        dimensions[ 0 ] = w;
        dimensions[ 1 ] = h;
        
        for( int i = 0; i < tabs.size(); ++i )
        {
            tabs[ i ].contents -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
            tabs[ i ].contents -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        }
        
        parent.requestRedraw();
    }
    
    void tabset::addTab( group* g, std::string t )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        g -> setRealPosition( position[ 0 ], position[ 1 ] + TABSET_BAR_HEIGHT );
        g -> setRealDimensions( dimensions[ 0 ], dimensions[ 1 ] - TABSET_BAR_HEIGHT );
        
        tab_data new_data;
        
        new_data.contents = g;
        new_data.title = t;
        new_data.state = tab_data::CLOSE_SAFE;
        new_data.button_state = tab_data::UP;
        new_data.position = 0;
        new_data.width = TABSET_MIN_TAB_WIDTH;
        
        tabs.push_back( new_data );
        
        if( current_tab < 0 )
            current_tab = 0;
        
        reorganizeTabs();                                                       // Calls parent.requestRedraw()
    }
    void tabset::removeTab( group* g )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        for( std::vector< tab_data >::iterator iter = tabs.begin();
             iter != tabs.end();
             ++iter )
        {
            if( iter -> contents == g )
            {
                if( tabs[ current_tab ].contents == g
                    && current_tab == tabs.size() - 1 )                         // Select a new current tab as the one to the right if it exists, otherwise to
                                                                                // the left (which implicity selects none if there are no more tabs)
                {
                    --current_tab;
                    
                    if( current_tab >= 0 )
                        tabs[ current_tab ].contents -> shown();
                }
                
                tabs.erase( iter );
                
                reorganizeTabs();                                               // Calls parent.requestRedraw()
                
                return;
            }
        }
        
        throw exception( "tabset::removeTab(): No such tab" );
    }
    
    bool tabset::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock( element_lock, RW_WRITE );
        
        if( e.type == STROKE )
        {
            if( capturing )
            {
                if( !( e.stroke.click & CLICK_PRIMARY ) )                       // Capture cancelled
                {
                    capturing = false;
                    parent.associateDevice( e.stroke.dev_id, NULL );
                    return true;                                                // Accept event because we used it
                }
                else
                {
                    
                }
            }
            else
            {
                if( ( e.stroke.click & CLICK_PRIMARY )
                    && pointInsideRect( e.stroke.position[ 0 ],
                                        e.stroke.position[ 1 ],
                                        position[ 0 ],
                                        position[ 1 ],
                                        dimensions[ 0 ],
                                        TABSET_BAR_HEIGHT ) )                   // Get event if in BAR
                {
                    for( int i = 0; i < tabs.size(); ++ i )
                    {
                        if( pointInsideRect( e.stroke.position[ 0 ],
                                             e.stroke.position[ 1 ],
                                             tabs[ i ].position + bar_scroll,
                                             position[ 1 ],
                                             tabs[ i ].width,
                                             TABSET_TAB_HEIGHT ) )              // Click if on TAB
                        {
                            if( i != current_tab )
                            {
                                current_tab = i;
                                parent.requestRedraw();
                            }
                            
                            // capturing = true;
                            // capture_start[ 0 ] = e.stroke.position[ 0 ];
                            // capture_start[ 1 ] = e.stroke.position[ 1 ];
                            
                            // parent.associateDevice( e.stroke.dev_id, this );
                            
                            return true;
                        }
                    }
                    
                    return !event_fallthrough;
                }
                
                // Let event fall down to current tabs' contents
            }
        }
        else
        {
            if( e.type == SCROLL
                && pointInsideRect( e.scroll.position[ 0 ],
                                    e.scroll.position[ 1 ],
                                    position[ 0 ],
                                    position[ 1 ],
                                    dimensions[ 0 ],
                                    TABSET_BAR_HEIGHT ) )
            {
                if( e.scroll.amount[ 0 ] != 0.0f )
                    bar_scroll += e.scroll.amount[ 0 ] * TABSET_SCROLL_FACTOR;
                else
                    bar_scroll += e.scroll.amount[ 1 ] * TABSET_SCROLL_FACTOR;
                
                reorganizeTabs();
            }
        }
        
        if( current_tab >= 0 )                                                  // current_tab is a group so it will convert event position for us
            return tabs[ current_tab ].contents -> acceptEvent( e ) || !event_fallthrough;
        else
            return !event_fallthrough;
    }
    
    void tabset::draw()
    {
        scoped_lock< rwlock > slock_e( element_lock, RW_READ );
        scoped_lock< rwlock > slock_r( tabset_rsrc_lock, RW_READ );
        
        tabset_set& set = tabset_sets[ &parent ];
        
        glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
        {
            glPushMatrix();
            {
                glScalef( dimensions[ 0 ], 1.0f, 1.0f );
                
                set.fill -> draw();
            }
            glPopMatrix();
            
            glPushMatrix();
            {
                glTranslatef( bar_scroll, 0.0f, 0.0f );
                
                for( int i = 0; i < tabs.size(); ++i )
                {
                    tab_set* t_set;
                    ctrl_set* c_set;
                    
                    if( i == current_tab )
                        t_set = &set.active;
                    else
                        t_set = &set.inactive;
                    
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
                        glTranslatef( -14.0f, 0.0f, 0.0f );
                        
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
                    
                    glTranslatef( 6.0f, 0.0f, 0.0f );
                }
            }
            glPopMatrix();
            
            glTranslatef( position[ 0 ], position[ 1 ], 0.0f );
            {
                if( current_tab >= 0 )
                    tabs[ current_tab ].contents -> draw();
            }
            glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
        }
        glTranslatef( position[ 0 ] * -1.0f, position[ 1 ] * -1.0f, 0.0f );
    }
}


