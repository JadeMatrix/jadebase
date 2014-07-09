/* 
 * bqt_window.cpp
 * 
 * About
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_window.hpp"

#include "bqt_exception.hpp"
#include "bqt_log.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    bqt::window_id getNewWindowID()                                             // Need to be thread-safe?
    {
        static bqt::window_id last_id = 0x00;
        return last_id++;
    }
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // WINDOW //////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        platform_window.sdl_window = SDL_CreateWindow( title.c_str(),
                                                       SDL_WINDOWPOS_CENTERED,
                                                       SDL_WINDOWPOS_CENTERED,
                                                       dimensions[ 0 ],
                                                       dimensions[ 1 ],
                                                       SDL_WINDOW_OPENGL );
        
        
        if( platform_window.sdl_window == NULL )
            throw exception( "bqt::window::init(): Could not create SDL window" );
        
        platform_window.sdl_gl_context = SDL_GL_CreateContext( platform_window.sdl_window );
    }
    
    window::window( window_id id ) : id( id )
    {
        platform_window.sdl_window = NULL;
        
        title = BQT_WINDOW_DEFAULT_NAME;
        
        dimensions[ 0 ] = BQT_WINDOW_DEFAULT_WIDTH;
        dimensions[ 1 ] = BQT_WINDOW_DEFAULT_HEIGHT;
        
        fullscreen = false;
        
        updates.changed    = false;
        updates.close      = false;
        updates.dimensions = false;
        updates.position   = false;
        updates.fullscreen = false;
        updates.title      = false;
        updates.minimize   = false;
        updates.maximize   = false;
        updates.restore    = false;
    }
    window::~window()
    {
        if( platform_window.sdl_window != NULL )
        {
            SDL_GL_DeleteContext( platform_window.sdl_gl_context );
            SDL_DestroyWindow( platform_window.sdl_window );
        }
    }
    
    void window::makeCurrent()
    {
        scoped_lock slock( window_mutex );
        
        if( SDL_GL_MakeCurrent( platform_window.sdl_window, platform_window.sdl_gl_context ) )
            throw exception( "bqt::window::makeCurrent(): Could not make SDL GL context current" );
    }
    
    void window::addCanvas( canvas* c, view_id v, int t )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "bqt::window::addCanvas(): Not implemented" );
    }
    void window::removeCanvas( canvas* c )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "bqt::window::removeCanvas(): Not implemented" );
    }
    
    void window::setToolVisibility( bool v )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "bqt::window::setToolVisibility(): Not implemented" );
    }
    void window::setViewZoom( view_id v, float z )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "bqt::window::setViewZoom(): Not implemented" );
    }
    
    std::pair< unsigned int, unsigned int > window::getDimensions()
    {
        scoped_lock slock( window_mutex );
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< unsigned int, unsigned int > window::getPosition()
    {
        scoped_lock slock( window_mutex );
        return std::pair< unsigned int, unsigned int >( position[ 0 ], position[ 1 ] );
    }
    
    float window::getViewZoom( view_id v )
    {
        scoped_lock slock( window_mutex );
        return view_zoom;
    }
    
    void window::acceptEvent( event& e )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "bqt::window::acceptEvent(): Not implemented" );
    }
    
    // WINDOW::MANIPULATE //////////////////////////////////////////////////////
    
    window::manipulate::manipulate( window* t )
    {
        if( t == NULL )
        {
            target = new window( getNewWindowID() );
            // TODO: register with manager
        }
        else
            target = t;
    }
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        scoped_lock slock( target -> window_mutex );
        
        SDL_Window*& sdl_window = target -> platform_window.sdl_window;
        
        if( sdl_window == NULL )
            target -> init();
        
        if( target -> updates.close )
            delete target;
        else
        {
            if( target -> updates.changed )
            {
                if( target -> updates.dimensions )
                {
                    SDL_SetWindowSize( sdl_window,
                                       target -> dimensions[ 0 ],
                                       target -> dimensions[ 1 ] );
                    target -> updates.dimensions = false;
                }
                
                if( target -> updates.position )
                {
                    SDL_SetWindowPosition( sdl_window,
                                           target -> dimensions[ 0 ],
                                           target -> dimensions[ 1 ] );
                    target -> updates.position = false;
                }
                
                if( target -> updates.fullscreen )
                {
                    if( target -> fullscreen )
                        SDL_SetWindowFullscreen( sdl_window, SDL_TRUE );
                    else
                        SDL_SetWindowFullscreen( sdl_window, SDL_FALSE );
                    target -> updates.fullscreen = false;
                }
                
                if( target -> updates.title )
                {
                    SDL_SetWindowTitle( sdl_window, target -> title.c_str() );
                    target -> updates.title = false;
                }
                
                if( target -> updates.minimize )
                {
                    SDL_MinimizeWindow( sdl_window );
                    target -> updates.minimize = false;
                }
                
                if( target -> updates.maximize )
                {
                    // TODO: implement
                    throw exception( "bqt::window::manipulate::execute(): Window maximize not implemented" );
                    target -> updates.maximize = false;
                }
                
                if( target -> updates.restore )
                {
                    SDL_RestoreWindow( sdl_window );
                    target -> updates.restore = false;
                }
                
                target -> updates.changed = false;
            }
        }
        
        return true;
    }
    
    void window::manipulate::setDimensions( unsigned int w, unsigned int h )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> dimensions[ 0 ] = w;
        target -> dimensions[ 1 ] = h;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( unsigned int x, unsigned int y )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> position[ 0 ] = x;
        target -> position[ 1 ] = y;
        target -> updates.position = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::setFullscreen( bool f )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> fullscreen = true;
        target -> updates.fullscreen = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setTitle( std::string t )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> title = t;
        target -> updates.title = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::minimize()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.minimize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::maximize()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.maximize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::restore()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.restore = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::close()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.close = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::dropCanvas( canvas* c, unsigned int x, unsigned int y )
    {
        scoped_lock slock( target -> window_mutex );
        // TODO: implement
        throw exception( "bqt::window::manipulate::dropCanvas(): Not implemented" );
    }
}


