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
#include "bqt_windowmanagement.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_gl.hpp"
#include "bqt_preferences.hpp"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    // WINDOW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        platform_window.sdl_window = SDL_CreateWindow( title.c_str(),
                                                       SDL_WINDOWPOS_CENTERED,
                                                       SDL_WINDOWPOS_CENTERED,
                                                       dimensions[ 0 ],
                                                       dimensions[ 1 ],
                                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
        
        
        if( platform_window.sdl_window == NULL )
            throw exception( "window::init(): Could not create SDL window" );
        
        platform_window.sdl_gl_context = SDL_GL_CreateContext( platform_window.sdl_window );
        
        registerWindow( *this );
    }
    
    window::window()
    {
        platform_window.sdl_window = NULL;
        
        pending_redraws = 0;
        
        title = BQT_WINDOW_DEFAULT_NAME;
        
        dimensions[ 0 ] = BQT_WINDOW_DEFAULT_WIDTH;
        dimensions[ 1 ] = BQT_WINDOW_DEFAULT_HEIGHT;
        
        fullscreen = false;
        in_focus = false;
        
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
    
    void window::addCanvas( canvas* c, view_id v, int t )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::addCanvas(): Not implemented" );
    }
    void window::removeCanvas( canvas* c )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::removeCanvas(): Not implemented" );
    }
    
    void window::setToolVisibility( bool v )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::setToolVisibility(): Not implemented" );
    }
    void window::setViewZoom( view_id v, float z )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::setViewZoom(): Not implemented" );
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
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock slock( window_mutex );
        // TODO: implement
        throw exception( "window::acceptEvent(): Not implemented" );
    }
    
    bqt_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock slock( window_mutex );
        
        if( platform_window.sdl_window == NULL )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    // WINDOW::MANIPULATE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::manipulate::manipulate( window* t )
    {
        if( t == NULL )
        {
            target = new window();
        }
        else
            target = t;
    }
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        bool redraw_window = false;
        
        target -> window_mutex.lock();                                          // We need to explicitly lock/unlock this as the window can be destroyed
        
        SDL_Window*& sdl_window = target -> platform_window.sdl_window;
        
        if( sdl_window == NULL )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( target -> updates.close )
        {
            deregisterWindow( *target );
            target -> window_mutex.unlock();
            delete target;
            
            if( getQuitOnNoWindows() && getRegisteredWindowCount() < 1 )
            {
                SDL_Event* sdl_event = new SDL_Event;
                sdl_event -> type = SDL_QUIT;
                
                if( SDL_PushEvent( sdl_event ) < 1 )
                {
                    ff::write( bqt_out, "Warning: Failed to push a quit event internally\n" );
                    delete sdl_event;
                }                                                               // If pushed, we let the allocated memory hang since it's not clear what SDL
                                                                                // does with it.
            }
        }
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
                    SDL_MaximizeWindow( sdl_window );
                    target -> updates.maximize = false;
                }
                
                if( target -> updates.restore )
                {
                    SDL_RestoreWindow( sdl_window );
                    target -> updates.restore = false;
                }
                
                target -> updates.changed = false;
                
                redraw_window = true;
            }
            
            target -> window_mutex.unlock();
        }
        
        if( redraw_window )
            submitTask( new window::redraw( *target ) );
        
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
    
    void window::manipulate::setFocus( bool f )
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> in_focus = true;
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
    
    void window::manipulate::redraw()
    {
        scoped_lock slock( target -> window_mutex );
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::dropCanvas( canvas* c, unsigned int x, unsigned int y )
    {
        scoped_lock slock( target -> window_mutex );
        // TODO: implement
        throw exception( "window::manipulate::dropCanvas(): Not implemented" );
    }
    
    // WINDOW::REDRAW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock slock( target.window_mutex );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        scoped_lock slock( target.window_mutex );
        
        if( target.pending_redraws <= 1 )                                       // Only redraw if there are no other pending redraws for that window; this is
                                                                                // safe because the redraw task is high-priority, so the task system will
                                                                                // eventually drill down to the last one.
        {
            if( target.pending_redraws != 1 )                                   // Sanity check
                throw exception( "window::redraw::execute(): Target pending redraws somehow < 1" );
            
            if( SDL_GL_MakeCurrent( target.platform_window.sdl_window, target.platform_window.sdl_gl_context ) )
                throw exception( "window::redraw::execute(): Could not make SDL GL context current" );
            
            // TODO: Implement
            {
                glEnable(GL_DEPTH_TEST);
                glViewport( 0, 0, target.dimensions[ 0 ], target.dimensions[ 1 ] );
                
                glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                
                
            }
            
            SDL_GL_SwapWindow( target.platform_window.sdl_window );
        }
        
        target.pending_redraws--;
        
        return true;
    }
}


