/* 
 * bqt_window.cpp
 * 
 * Implements bqt_window.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_window.hpp"

#include <cmath>

#include "bqt_exception.hpp"
#include "bqt_log.hpp"
#include "bqt_windowmanagement.hpp"
#include "bqt_taskexec.hpp"
#include "bqt_preferences.hpp"
#include "bqt_launchargs.hpp"
#include "bqt_gl.hpp"
#include "bqt_png.hpp"
#include "gui/bqt_gui_element.hpp"
#include "gui/bqt_gui_image_rsrc.hpp"

#include "gui/bqt_gui_button.hpp"
#include "gui/bqt_gui_dial.hpp"
#include "gui/bqt_gui_tabset.hpp"
#include "gui/bqt_gui_group.hpp"

/******************************************************************************//******************************************************************************/

#if DEBUG
#define GUI_RESOURCE_FILE "make/BQTDraw/Resources/gui_resources.png"
#else
#define GUI_RESOURCE_FILE "Resources/gui_resources.png"
#endif

namespace bqt
{
    // WINDOW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void window::init()
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        #if defined PLATFORM_XWS_GNUPOSIX
        
        Display* x_display = getXDisplay();
        Window x_root = DefaultRootWindow( x_display );
        
        XMapWindow( x_display, x_root );
        
        XFlush( x_display );
        
        platform_window.x_visual_info = glXChooseVisual( x_display,
                                                         DefaultScreen( x_display ),
                                                         platform_window.glx_attr );
        
        if( platform_window.x_visual_info == NULL )
            throw exception( "window::init(): No appropriate X visual found" );
        
        // TODO: Do we need to initialize platform_window.x_set_window_att?
        platform_window.x_set_window_attr.colormap = XCreateColormap( x_display,
                                                                      x_root,
                                                                      platform_window.x_visual_info -> visual,
                                                                      AllocNone );
        platform_window.x_set_window_attr.event_mask = KeyPressMask
                                                       | KeyReleaseMask
                                                       | ButtonPressMask
                                                       | ButtonReleaseMask
                                                       | EnterWindowMask
                                                       | LeaveWindowMask
                                                       | PointerMotionMask
                                                       // | PointerMotionHintMask
                                                       // | Button1MotionMask
                                                       // | Button2MotionMask
                                                       // | Button3MotionMask
                                                       // | Button4MotionMask
                                                       // | Button5MotionMask
                                                       | ButtonMotionMask
                                                       // | KeyMapStateMask
                                                       | ExposureMask
                                                       | VisibilityChangeMask
                                                       | StructureNotifyMask
                                                       // | ResizeRedirectMask
                                                       | SubstructureNotifyMask
                                                       // | SubstructureRedirectMask
                                                       | FocusChangeMask
                                                       | PropertyChangeMask
                                                       | ColormapChangeMask
                                                       // | OwnerGrabButtonMask
                                                       ;
        
        platform_window.x_window = XCreateWindow( x_display,
                                                  x_root,
                                                  position[ 0 ],
                                                  position[ 1 ],
                                                  dimensions[ 0 ],
                                                  dimensions[ 1 ],
                                                  0,                            // Window border width, unused
                                                  platform_window.x_visual_info -> depth,
                                                  InputOutput,
                                                  platform_window.x_visual_info -> visual,
                                                  CWColormap | CWEventMask,
                                                  &platform_window.x_set_window_attr );
        
        XMapWindow( x_display, platform_window.x_window );
        XStoreName( x_display, platform_window.x_window, title.c_str() );
        
        platform_window.x_protocols[ 0 ] = XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False );
        if( !XSetWMProtocols( x_display,
                              platform_window.x_window,
                              platform_window.x_protocols,
                              X_PROTOCOL_COUNT ) )
            throw exception( "window::init(): Could not set X window protocols" );
        
        XFlush( x_display );                                                    // We need to flush X before creating the GLX Context
        
        platform_window.glx_context = glXCreateContext( x_display,
                                                        platform_window.x_visual_info,
                                                        NULL,
                                                        GL_TRUE );
        glXMakeCurrent( x_display, platform_window.x_window, platform_window.glx_context );
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
        
        platform_window.good = true;
        
        // TODO: remove
        GLenum err = glewInit();
        if( err != GLEW_OK )
        {
            bqt::exception e;
            ff::write( *e, "Failed to initialize GLEW: ", std::string( ( const char* )glewGetErrorString( err ) ) );
            throw e;
        }
        
        registerWindow( *this );
        
        initOpenGL();
        
        initNamedResources();
        
        // Devel
        {
            group* test_group_a = new group( *this, 0, 0, 256, 256 );
            group* test_group_b = new group( *this, 0, 0, 256, 256 );
            
            test_group_a -> addElement( new button( *this, 10, 10, 60, 40 ) );
            test_group_a -> addElement( new button( *this, 72, 10, 26, 40 ) );
            test_group_a -> addElement( new button( *this, 100, 10, 40, 19 ) );
            test_group_a -> addElement( new button( *this, 100, 31, 40, 19 ) );
            
            test_group_b -> addElement( new dial( *this, 10, 70 ) );
            test_group_b -> addElement( new dial( *this, 62, 70, true ) );
            
            tabset* test_tabset = new tabset( *this, 0, 0, 256, 256 );
            
            test_tabset -> addTab( test_group_a, "test tab A" );
            test_tabset -> addTab( test_group_b, "test tab B" );
            
            elements.push_back( test_tabset );
            
            requestRedraw();
        }
    }
    
    void window::makeContextCurrent()
    {
        Display* x_display = getXDisplay();
        glXMakeCurrent( x_display,
                        platform_window.x_window,
                        platform_window.glx_context );
        
        XFlush( x_display );
    }
    
    void window::initNamedResources()
    {
        named_resources[ rounded_button_off_up_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 0, 6, 7 );
        named_resources[ rounded_button_off_up_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 0, 1, 7 );
        named_resources[ rounded_button_off_up_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 0, 6, 7 );
        named_resources[ rounded_button_off_up_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 7, 6, 1 );
        named_resources[ rounded_button_off_up_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 7, 1, 1 );
        named_resources[ rounded_button_off_up_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 7, 6, 1 );
        named_resources[ rounded_button_off_up_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 8, 6, 7 );
        named_resources[ rounded_button_off_up_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 8, 1, 7 );
        named_resources[ rounded_button_off_up_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 8, 6, 7 );
        named_resources[ rounded_button_off_down_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 15, 6, 7 );
        named_resources[ rounded_button_off_down_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 15, 1, 7 );
        named_resources[ rounded_button_off_down_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 15, 6, 7 );
        named_resources[ rounded_button_off_down_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 22, 6, 1 );
        named_resources[ rounded_button_off_down_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 22, 1, 1 );
        named_resources[ rounded_button_off_down_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 22, 6, 1 );
        named_resources[ rounded_button_off_down_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 23, 6, 7 );
        named_resources[ rounded_button_off_down_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 23, 1, 7 );
        named_resources[ rounded_button_off_down_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 23, 6, 7 );
        named_resources[ rounded_button_on_up_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 30, 6, 7 );
        named_resources[ rounded_button_on_up_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 30, 1, 7 );
        named_resources[ rounded_button_on_up_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 30, 6, 7 );
        named_resources[ rounded_button_on_up_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 37, 6, 1 );
        named_resources[ rounded_button_on_up_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 37, 1, 1 );
        named_resources[ rounded_button_on_up_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 37, 6, 1 );
        named_resources[ rounded_button_on_up_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 38, 6, 7 );
        named_resources[ rounded_button_on_up_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 38, 1, 7 );
        named_resources[ rounded_button_on_up_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 38, 6, 7 );
        named_resources[ rounded_button_on_down_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 45, 6, 7 );
        named_resources[ rounded_button_on_down_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 45, 1, 7 );
        named_resources[ rounded_button_on_down_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 45, 6, 7 );
        named_resources[ rounded_button_on_down_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 52, 6, 1 );
        named_resources[ rounded_button_on_down_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 52, 1, 1 );
        named_resources[ rounded_button_on_down_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 52, 6, 1 );
        named_resources[ rounded_button_on_down_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 53, 6, 7 );
        named_resources[ rounded_button_on_down_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 53, 1, 7 );
        named_resources[ rounded_button_on_down_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 53, 6, 7 );
        named_resources[ squared_button_off_up_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 0, 6, 7 );
        named_resources[ squared_button_off_up_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 0, 1, 7 );
        named_resources[ squared_button_off_up_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 0, 6, 7 );
        named_resources[ squared_button_off_up_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 7, 6, 1 );
        named_resources[ squared_button_off_up_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 7, 1, 1 );
        named_resources[ squared_button_off_up_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 7, 6, 1 );
        named_resources[ squared_button_off_up_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 8, 6, 7 );
        named_resources[ squared_button_off_up_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 8, 1, 7 );
        named_resources[ squared_button_off_up_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 8, 6, 7 );
        named_resources[ squared_button_off_down_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 15, 6, 7 );
        named_resources[ squared_button_off_down_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 15, 1, 7 );
        named_resources[ squared_button_off_down_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 15, 6, 7 );
        named_resources[ squared_button_off_down_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 22, 6, 1 );
        named_resources[ squared_button_off_down_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 22, 1, 1 );
        named_resources[ squared_button_off_down_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 22, 6, 1 );
        named_resources[ squared_button_off_down_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 23, 6, 7 );
        named_resources[ squared_button_off_down_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 23, 1, 7 );
        named_resources[ squared_button_off_down_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 23, 6, 7 );
        named_resources[ squared_button_on_up_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 30, 6, 7 );
        named_resources[ squared_button_on_up_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 30, 1, 7 );
        named_resources[ squared_button_on_up_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 30, 6, 7 );
        named_resources[ squared_button_on_up_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 37, 6, 1 );
        named_resources[ squared_button_on_up_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 37, 1, 1 );
        named_resources[ squared_button_on_up_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 37, 6, 1 );
        named_resources[ squared_button_on_up_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 38, 6, 7 );
        named_resources[ squared_button_on_up_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 38, 1, 7 );
        named_resources[ squared_button_on_up_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 38, 6, 7 );
        named_resources[ squared_button_on_down_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 45, 6, 7 );
        named_resources[ squared_button_on_down_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 45, 1, 7 );
        named_resources[ squared_button_on_down_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 45, 6, 7 );
        named_resources[ squared_button_on_down_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 52, 6, 1 );
        named_resources[ squared_button_on_down_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 52, 1, 1 );
        named_resources[ squared_button_on_down_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 52, 6, 1 );
        named_resources[ squared_button_on_down_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 53, 6, 7 );
        named_resources[ squared_button_on_down_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 19, 53, 1, 7 );
        named_resources[ squared_button_on_down_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 53, 6, 7 );
        named_resources[ tab_active_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 60, 6, 30 );
        named_resources[ tab_active_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 6, 60, 1, 30 );
        named_resources[ tab_active_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 7, 60, 6, 30 );
        named_resources[ tab_inactive_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 14, 60, 6, 30 );
        named_resources[ tab_inactive_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 20, 60, 1, 30 );
        named_resources[ tab_inactive_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 21, 60, 6, 30 );
        named_resources[ tab_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 13, 60, 1, 30 );
        named_resources[ tab_control_active_close_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 38, 110, 20, 23 );
        named_resources[ tab_control_active_close_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 58, 110, 20, 23 );
        named_resources[ tab_control_active_close_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 78, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 98, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 118, 110, 20, 23 );
        named_resources[ tab_control_active_unsaved_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 128, 110, 20, 23 );
        named_resources[ tab_control_inactive_close_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 38, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 58, 133, 20, 23 );
        named_resources[ tab_control_inactive_close_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 78, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 98, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 118, 133, 20, 23 );
        named_resources[ tab_control_inactive_unsaved_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 128, 133, 20, 23 );
        named_resources[ toolbox_close_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 26, 0, 17, 17 );
        named_resources[ toolbox_close_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 26, 17, 17, 17 );
        named_resources[ toolbox_close_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 26, 34, 17, 17 );
        named_resources[ toolbox_collapse_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 43, 0, 17, 17 );
        named_resources[ toolbox_collapse_over ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 43, 17, 17, 17 );
        named_resources[ toolbox_collapse_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 43, 34, 17, 17 );
        named_resources[ toolbox_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 27, 51, 1, 17 );
        named_resources[ ruler_corner_below ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 28, 51, 16, 16 );
        named_resources[ ruler_corner_above ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 44, 51, 16, 16 );
        named_resources[ ruler_mark_inside_large ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 63, 51, 1, 15 );
        named_resources[ ruler_mark_inside_medium ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 64, 51, 1, 15 );
        named_resources[ ruler_mark_inside_small ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 65, 51, 1, 15 );
        named_resources[ ruler_mark_outside_large ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 60, 51, 1, 15 );
        named_resources[ ruler_mark_outside_medium ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 61, 51, 1, 15 );
        named_resources[ ruler_mark_outside_small ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 62, 51, 1, 15 );
        named_resources[ ruler_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 66, 51, 1, 15 );
        named_resources[ divider ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 60, 42, 1, 1 );
        named_resources[ scrollbar_button_left_bottom_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 189, 9, 25, 12 );
        named_resources[ scrollbar_button_left_bottom_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 189, 12, 25, 12 );
        named_resources[ scrollbar_button_right_top_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 150, 0, 25, 12 );
        named_resources[ scrollbar_button_right_top_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 150, 12, 25, 12 );
        named_resources[ scrollbar_bar_left_bottom_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 183, 0, 6, 12 );
        named_resources[ scrollbar_bar_center_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 182, 0, 1, 12 );
        named_resources[ scrollbar_bar_right_top_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 176, 0, 6, 12 );
        named_resources[ scrollbar_bar_left_bottom_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 183, 12, 6, 12 );
        named_resources[ scrollbar_bar_center_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 182, 12, 1, 12 );
        named_resources[ scrollbar_bar_right_top_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 176, 12, 6, 12 );
        named_resources[ scrollbar_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 175, 0, 1, 12 );
        named_resources[ scrollbar_corner_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 214, 0, 12, 12 );
        named_resources[ scrollbar_corner_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 214, 12, 12, 12 );
        named_resources[ scrollbar_corner_evil ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 226, 0, 12, 12 );
        named_resources[ viewbar_handle ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 88, 68, 20, 8 );
        named_resources[ viewbar_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 87, 68, 1, 8 );
        named_resources[ viewbar_corner ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 108, 68, 8, 8 );
        named_resources[ dropdown_button_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 67, 20, 21, 22 );
        named_resources[ dropdown_button_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 88, 20, 21, 22 );
        named_resources[ dropdown_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 60, 20, 6, 22 );
        named_resources[ dropdown_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 66, 20, 1, 22 );
        named_resources[ radio_off_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 60, 0, 14, 20 );
        named_resources[ radio_off_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 74, 0, 14, 20 );
        named_resources[ radio_on_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 88, 0, 14, 20 );
        named_resources[ radio_on_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 102, 0, 14, 20 );
        named_resources[ buttonmenu_top_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 90, 22, 15 );
        named_resources[ buttonmenu_top_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 22, 90, 1, 15 );
        named_resources[ buttonmenu_top_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 23, 90, 15, 15 );
        named_resources[ buttonmenu_center_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 105, 22, 1 );
        named_resources[ buttonmenu_center_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 22, 105, 1, 1 );
        named_resources[ buttonmenu_center_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 23, 105, 15, 1 );
        named_resources[ buttonmenu_bottom_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 141, 22, 15 );
        named_resources[ buttonmenu_bottom_center ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 22, 141, 1, 15 );
        named_resources[ buttonmenu_bottom_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 23, 141, 15, 15 );
        named_resources[ buttonmenu_arrow ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 106, 22, 35 );
        named_resources[ checkbox_off_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 116, 0, 17, 20 );
        named_resources[ checkbox_off_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 133, 0, 17, 20 );
        named_resources[ checkbox_part_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 116, 20, 17, 20 );
        named_resources[ checkbox_part_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 116, 40, 17, 20 );
        named_resources[ checkbox_on_up ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 133, 20, 17, 20 );
        named_resources[ checkbox_on_down ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 133, 40, 17, 20 );
        named_resources[ slider_empty_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 30, 44, 2, 4 );
        named_resources[ slider_empty_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 29, 44, 1, 4 );
        named_resources[ slider_empty_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 27, 44, 2, 4 );
        named_resources[ slider_full_left ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 30, 48, 2, 4 );
        named_resources[ slider_full_fill ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 29, 48, 1, 4 );
        named_resources[ slider_full_right ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 27, 48, 2, 4 );
        named_resources[ dial_large_dial ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 156, 42, 42 );
        named_resources[ dial_large_dot ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 42, 156, 6, 6 );
        named_resources[ dial_small_dial ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 0, 198, 22, 22 );
        named_resources[ dial_small_dot ] = new image_rsrc( *this, GUI_RESOURCE_FILE, 42, 162, 4, 4 );
    }
    
    void window::openUnopenedTextureFiles()
    {
        if( new_textures )
        {
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 ++iter )
            {
                if( iter -> second -> data == NULL
                    && iter -> second -> texture -> gl_texture == 0x00 )
                {
                    png_file rsrc_file( iter -> first );
                    
                    std::pair< unsigned int, unsigned int > rsrc_dim = rsrc_file.getDimensions();
                    
                    iter -> second -> data = new unsigned char[ rsrc_dim.first * rsrc_dim.second * 4 ];
                    
                    if( iter -> second -> data == NULL )
                        throw bqt::exception( "window::openUnopenedTextureFiles(): Could not allocate conversion space" );
                    
                    rsrc_file.toRGBABytes( iter -> second -> data );
                    
                    iter -> second -> texture -> dimensions[ 0 ] = rsrc_dim.first;
                    iter -> second -> texture -> dimensions[ 1 ] = rsrc_dim.second;
                    
                    // ff::write( bqt_out,
                    //            "Opening \"",
                    //            iter -> first,
                    //            "\" as a resource texture\n" );
                }
            }
        }
    }
    void window::uploadUnuploadedTextures()
    {
        if( new_textures )
        {
            makeContextCurrent();
            
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 ++iter )
            {
                if( iter -> second -> texture -> gl_texture == 0x00
                    && iter -> second -> data != NULL )
                {
                    glGenTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                    
                    if( iter -> second -> texture -> gl_texture == 0x00 )
                        throw exception( "window::uploadUnuploadedTextures(): Could not generate texture" );
                    
                    glBindTexture( GL_TEXTURE_2D, iter -> second -> texture -> gl_texture );
                    glTexImage2D( GL_TEXTURE_2D,
                                  0,
                                  GL_RGBA,
                                  iter -> second -> texture -> dimensions[ 1 ],
                                  iter -> second -> texture -> dimensions[ 0 ],
                                  0,
                                  GL_RGBA,
                                  GL_UNSIGNED_BYTE,
                                  iter -> second -> data );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                    
                    glBindTexture( GL_TEXTURE_2D, 0 );
                    
                    GLenum gl_error = glGetError();
                    if( gl_error != GL_NO_ERROR )
                    {
                        bqt::exception e;
                        ff::write( *e,
                                   "LoadGUIResource_task::execute(): OpenGL error 0x",
                                   ff::to_x( ( unsigned long )gl_error ),
                                   " loading pixels from 0x",
                                   ff::to_x( ( unsigned long )( iter -> second -> data ), HEX_WIDTH, HEX_WIDTH ),
                                   " to texture 0x",
                                   ff::to_x( iter -> second -> texture -> gl_texture, HEX_WIDTH, HEX_WIDTH ) );
                        throw e;
                    }
                    
                    // ff::write( bqt_out,
                    //            "Creating \"",
                    //            iter -> first,
                    //            "\" as a resource texture 0x",
                    //            ff::to_x( iter -> second -> texture -> gl_texture ),
                    //            "\n" );
                    
                    delete[] iter -> second -> data;
                    iter -> second -> data = NULL;
                }
            }
            
            new_textures = false;
        }
    }
    void window::deleteUnreferencedTextures()
    {
        if( old_textures )
        {
            makeContextCurrent();
            
            for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
                 iter != resource_textures.end();
                 /* NULL */ )
            {
                if( iter -> second -> ref_count < 1 )
                {
                    // ff::write( bqt_out,
                    //            "Deleting \"",
                    //            iter -> first,
                    //            "\" as a resource texture 0x",
                    //            ff::to_x( iter -> second -> texture -> gl_texture ),
                    //            "\n" );
                    
                    if( iter -> second -> texture -> gl_texture != 0x00 )
                        glDeleteTextures( 1, &( iter -> second -> texture -> gl_texture ) );
                    
                    delete iter -> second;
                    
                    resource_textures.erase( iter++ );
                }
                else
                    ++iter;
            }
            
            old_textures = false;
        }
    }
    
    window::~window()
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );                   // If we really need this we have bigger problems
        
        #if defined PLATFORM_XWS_GNUPOSIX
        
        if( platform_window.good )
        {
            Display* x_display = getXDisplay();
            
            glXMakeCurrent( x_display, None, NULL );
            glXDestroyContext( x_display, platform_window.glx_context );
            XDestroyWindow( x_display, platform_window.x_window );
            
            platform_window.good = false;
        }
        
        #else
        
        #error "Windows not implemented on non-X platforms"
        
        #endif
    }
    
    window::window() : input_assoc( bqt_platform_idevid_t_comp )
    {
        platform_window.good = false;
        
        platform_window.glx_attr[ 0 ] = GLX_RGBA;
        platform_window.glx_attr[ 1 ] = GLX_DEPTH_SIZE;
        platform_window.glx_attr[ 2 ] = 24;
        platform_window.glx_attr[ 3 ] = GLX_DOUBLEBUFFER;
        platform_window.glx_attr[ 4 ] = None;
        
        // platform_window.sdl_window = NULL;
        
        pending_redraws = 0;
        
        title = BQT_WINDOW_DEFAULT_NAME;
        
        dimensions[ 0 ] = BQT_WINDOW_DEFAULT_WIDTH;
        dimensions[ 1 ] = BQT_WINDOW_DEFAULT_HEIGHT;
        position[ 0 ] = 0;
        position[ 1 ] = 0;
        
        fullscreen = false;
        in_focus = true;
        
        updates.changed    = false;
        updates.close      = false;
        updates.dimensions = false;
        updates.position   = false;
        updates.fullscreen = false;
        updates.title      = false;
        updates.minimize   = false;
        updates.maximize   = false;
        updates.restore    = false;
        updates.redraw     = false;
        
        new_textures = false;
        old_textures = false;
    }
    
    std::pair< unsigned int, unsigned int > window::getDimensions()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        return std::pair< unsigned int, unsigned int >( dimensions[ 0 ], dimensions[ 1 ] );
    }
    std::pair< int, int > window::getPosition()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        return std::pair< int, int >( position[ 0 ], position[ 1 ] );
    }
    
    void window::acceptEvent( window_event& e )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        // Devel
        if( e.type == KEYCOMMAND && e.key.key == KEY_Q && e.key.cmd && e.key.up )
        {
            setQuitFlag();
            return;
        }
        
        bool no_position = false;
        
        std::pair< int, int > element_position;
        std::pair< unsigned int, unsigned int > element_dimensions;
        int e_position[ 2 ];
        
        e.offset[ 0 ] = 0;
        e.offset[ 1 ] = 0;
        
        switch( e.type )
        {
        case STROKE:
            e.stroke.position[ 0 ] -= position[ 0 ];
            e.stroke.position[ 1 ] -= position[ 1 ];
            e.stroke.prev_pos[ 0 ] -= position[ 0 ];
            e.stroke.prev_pos[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.stroke.position[ 0 ];
            e_position[ 1 ] = e.stroke.position[ 1 ];
            break;
        case DROP:
            e.drop.position[ 0 ] -= position[ 0 ];
            e.drop.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.drop.position[ 0 ];
            e_position[ 1 ] = e.drop.position[ 1 ];
            break;
        case KEYCOMMAND:
        case COMMAND:
        case TEXT:
            no_position = true;
            break;
        case PINCH:
            e.pinch.position[ 0 ] -= position[ 0 ];
            e.pinch.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.pinch.position[ 0 ];
            e_position[ 1 ] = e.pinch.position[ 1 ];
            break;
        case SCROLL:
            e.scroll.position[ 0 ] -= position[ 0 ];
            e.scroll.position[ 1 ] -= position[ 1 ];
            e_position[ 0 ] = e.scroll.position[ 0 ];
            e_position[ 1 ] = e.scroll.position[ 1 ];
            break;
        default:
            throw exception( "window::acceptEvent(): Unknown event type" );
            break;
        }
        
        if( e.type == STROKE
            && input_assoc.count( e.stroke.dev_id ) )
        {
            idev_assoc& assoc( input_assoc[ e.stroke.dev_id ] );
            
            e.offset[ 0 ] = assoc.offset[ 0 ];
            e.offset[ 1 ] = assoc.offset[ 1 ];
            
            assoc.element -> acceptEvent( e );
            
            return;
        }
        
        for( int i = elements.size() - 1; i >= 0; -- i )                        // Iterate newest (topmost) first
        {
            if( no_position )
            {
                if( elements[ i ] -> acceptEvent( e ) )
                    break;
            }
            else
            {
                element_position   = elements[ i ] -> getVisualPosition();
                element_dimensions = elements[ i ] -> getVisualDimensions();
                
                if( ( e.type == STROKE
                      && pointInsideRect( e.stroke.prev_pos[ 0 ],
                                          e.stroke.prev_pos[ 1 ],
                                          element_position.first,
                                          element_position.second,
                                          element_dimensions.first,
                                          element_dimensions.second ) )
                    || pointInsideRect( e_position[ 0 ],
                                        e_position[ 1 ],
                                        element_position.first,
                                        element_position.second,
                                        element_dimensions.first,
                                        element_dimensions.second ) )
                {
                    if( elements[ i ] -> acceptEvent( e ) )
                        break;
                }
            }
        }
    }
    
    bqt_platform_window_t& window::getPlatformWindow()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        
        if( !platform_window.good )
            throw exception( "window::getPlatformWindow(): Window does not have a platform window yet" );
        else
            return platform_window;
    }
    
    void window::associateDevice( bqt_platform_idevid_t dev_id,
                                  gui_element* element,
                                  float off_x,
                                  float off_y )
    {
        scoped_lock< rwlock > scoped_lock( window_lock, RW_WRITE );
        
        idev_assoc& assoc( input_assoc[ dev_id ] );
        
        if( element == NULL )
            throw exception( "window::associateDevice(): Attempt to associate a device with a NULL element" );
        
        assoc.element = element;
        assoc.offset[ 0 ] = off_x;
        assoc.offset[ 1 ] = off_y;
    }
    void window::deassociateDevice( bqt_platform_idevid_t dev_id )
    {
        scoped_lock< rwlock > scoped_lock( window_lock, RW_WRITE );
        
        if( !input_assoc.erase( dev_id ) && getDevMode() )
            ff::write( bqt_out, "Warning: Attempt to deassociate a non-associated device" );
    }
    
    gui_texture* window::acquireTexture( std::string filename )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        gui_texture_holder* h;
        
        if( resource_textures.count( filename ) )
            h = resource_textures[ filename ];
        else
        {
            h = new gui_texture_holder();
            resource_textures[ filename ] = h;
            new_textures = true;
        }
        
        h -> ref_count++;
        
        // ff::write( bqt_out,
        //            "Acquiring texture \"",
        //            filename,
        //            "\" as 0x",
        //            ff::to_x( ( unsigned long )( h -> texture ) ),
        //            " (rc ",
        //            h -> ref_count,
        //            ")\n" );
        
        return h -> texture;
    }
    void window::releaseTexture( gui_texture* t )
    {
        scoped_lock< rwlock > slock( window_lock, RW_WRITE );
        
        for( std::map< std::string, gui_texture_holder* >::iterator iter = resource_textures.begin();
             iter != resource_textures.end();
             ++iter )
        {
            if( iter -> second -> texture == t )
            {
                iter -> second -> ref_count--;
                
                // ff::write( bqt_out,
                //            "Releasing texture \"",
                //            iter -> first,
                //            "\" as 0x",
                //            ff::to_x( ( unsigned long )( iter -> second -> texture ) ),
                //            " (rc ",
                //            iter -> second -> ref_count,
                //            ")\n" );
                
                if( iter -> second -> ref_count < 1 )
                {
                    old_textures = true;
                    
                    if( iter -> second -> ref_count < 0 )
                        throw exception( "window::releaseTexture(): Texture reference count < 0" );
                }
                
                return;
            }
        }
        
        throw exception( "window::releaseTexture(): No such texture" );
    }
    
    gui_resource* window::getNamedResource( gui_resource_name name )
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        
        if( named_resources.count( name ) )
            return named_resources[ name ];
        else
        {
            exception e;
            ff::write( *e,
                       "window::getNamedResource(): No resource with name 0x",
                       ff::to_x( ( unsigned long )name ) );
            throw e;
        }
    }
    
    void window::requestRedraw()
    {
        scoped_lock< rwlock > slock( window_lock, RW_READ );
        
        if( pending_redraws < 1 )
        {
            submitTask( new redraw( *this ) );
        }
    }
    
    // WINDOW::MANIPULATE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::manipulate::manipulate( window* t )
    {
        if( t == NULL )
            target = new window();                                              // Useful for debugging, or if we just need a window no matter what dammit
        else
            target = t;
    }
    
    bool window::manipulate::execute( task_mask* caller_mask )
    {
        bool redraw_window = false;
        
        target -> window_lock.lock_write();                                     // We need to explicitly lock/unlock this as the window can be destroyed
        
        if( !( target -> platform_window.good ) )
        {
            target -> init();
            redraw_window = true;
        }
        
        if( target -> updates.close )
        {
            /* GUI CLEANUP ****************************************************//******************************************************************************/
            
            for( int i = 0; i < target -> elements.size(); ++i )                          // Deletes all gui elements
                delete target -> elements[ i ];
            
            target -> elements.clear();
            
            for( std::map< gui_resource_name, gui_resource* >::iterator iter = target -> named_resources.begin();
                iter != target -> named_resources.end();
                ++iter )
            {
                delete iter -> second;
            }
            
            target -> elements.clear();
            
            for( std::map< std::string, gui_texture_holder* >::iterator iter = target -> resource_textures.begin();
                 iter != target -> resource_textures.end();
                 ++iter )                                                       // We should not have to delete textures since deleting the window
                                                                                // deletes the OpenGL context
            {
                delete iter -> second;
            }
            
            target -> resource_textures.clear();
            
            /* WINDOW CLEANUP *************************************************//******************************************************************************/
            
            deregisterWindow( *target );
            target -> window_lock.unlock();
            delete target;
            
            if( getQuitOnNoWindows() && getRegisteredWindowCount() < 1 )
            {
                if( getDevMode() )
                    ff::write( bqt_out, "All windows closed, quitting\n" );
                
                setQuitFlag();
            }
        }
        else
        {
            target -> openUnopenedTextureFiles();
            
            if( target -> updates.changed )
            {
                Display* x_display = getXDisplay();
                
                if( target -> updates.dimensions )
                {
                    // Actual resizing handled by WM, this is just for bounds checking
                    bool retry = false;
                    unsigned int new_dimensions[ 2 ];
                    new_dimensions[ 0 ] = target -> dimensions[ 0 ];
                    new_dimensions[ 1 ] = target -> dimensions[ 1 ];
                    
                    if( target -> dimensions[ 0 ] < BQT_WINDOW_MIN_WIDTH )
                    {
                        new_dimensions[ 0 ] = BQT_WINDOW_MIN_WIDTH;
                        retry = true;
                    }
                    if( target -> dimensions[ 1 ] < BQT_WINDOW_MIN_HEIGHT )
                    {
                        new_dimensions[ 1 ] = BQT_WINDOW_MIN_HEIGHT;
                        retry = true;
                    }
                    
                    if( retry )
                    {
                        XResizeWindow( x_display,
                                       target -> platform_window.x_window,
                                       new_dimensions[ 0 ],
                                       new_dimensions[ 1 ] );                   // This will generate a new event that leads here agan; this is OK
                        
                        redraw_window = false;
                    }
                    else
                    {
                        redraw_window = true;
                    }
                    
                    target -> updates.dimensions = false;
                }
                
                if( target -> updates.active )
                {
                    // target -> updates.restore = true;
                    
                    makeWindowActive( target -> getPlatformWindow() );
                }
                
                if( target -> updates.position )
                {
                    // Actual moving handled by WM
                    // XMoveWindow( x_display,
                    //              target -> platform_window.x_window,
                    //              target -> position[ 0 ],
                    //              target -> position[ 1 ] );
                    
                    target -> updates.position = false;
                }
                
                if( target -> updates.fullscreen )
                {
                    ff::write( bqt_out, "window::manipulate::execute(): Fullscreen not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Fullscreen not implemented
                    
                    target -> updates.fullscreen = false;
                    redraw_window = true;
                }
                
                if( target -> updates.title )
                {
                    XStoreName( x_display,
                                target -> platform_window.x_window,
                                target -> title.c_str() );
                    
                    target -> updates.title = false;
                }
                
                if( target -> updates.center )
                {
                    ff::write( bqt_out, "window::manipulate::execute(): Centering not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Centering not implemented
                    
                    target -> updates.center = false;
                }
                
                if( target -> updates.minimize )
                {
                    XIconifyWindow( x_display,
                                    target -> platform_window.x_window,
                                    DefaultScreen( x_display ) );
                    
                    target -> updates.minimize = false;
                }
                
                if( target -> updates.maximize )
                {
                    ff::write( bqt_out, "window::manipulate::execute(): Maximize not implemented yet, ignoring\n" );
                    // TODO: implement
                    #warning window::manipulate::execute(): Maximize not implemented
                    
                    makeWindowActive( target -> getPlatformWindow() );
                    
                    target -> updates.maximize = false;
                    redraw_window = true;
                }
                
                if( target -> updates.restore )
                {
                    XMapWindow( x_display,
                                target -> platform_window.x_window );
                    
                    makeWindowActive( target -> getPlatformWindow() );
                    
                    target -> updates.restore = false;
                    redraw_window = true;
                }
                
                if( target -> updates.redraw )
                {
                    target -> updates.redraw = false;
                    
                    redraw_window = true;
                }
                
                target -> updates.changed = false;
                
                XFlush( x_display );
            }
            
            target -> window_lock.unlock();
            
            if( redraw_window )
                submitTask( new window::redraw( *target ) );
        }
        
        return true;
    }
    
    void window::manipulate::setDimensions( unsigned int w, unsigned int h )
    {
        if( w < 1 || h < 1 )
            throw exception( "window::manipulate::setDimensions(): Width or height < 1" );
        
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> dimensions[ 0 ] = w;
        target -> dimensions[ 1 ] = h;
        target -> updates.dimensions = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setPosition( int x, int y )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> position[ 0 ] = x;
        target -> position[ 1 ] = y;
        target -> updates.position = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::setFullscreen( bool f )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> fullscreen = true;
        target -> updates.fullscreen = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::setTitle( std::string t )
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> title = t;
        target -> updates.title = true;
        
        target -> updates.changed = true;
    }
    
    // void window::manipulate::setFocus( bool f )
    // {
    //     scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
    //     target -> in_focus = true;
    // }
    void window::manipulate::makeActive()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.active = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::center()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.center = true;                                        // Don't calculate here, as that code may be thread-dependent
        
        target -> updates.changed = true;
    }
    void window::manipulate::minimize()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.minimize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::maximize()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.maximize = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::restore()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.restore = true;
        
        target -> updates.changed = true;
    }
    void window::manipulate::close()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.close = true;
        
        target -> updates.changed = true;
    }
    
    void window::manipulate::redraw()
    {
        scoped_lock< rwlock > slock( target -> window_lock, RW_WRITE );
        
        target -> updates.redraw = true;
        
        target -> updates.changed = true;
    }
    
    // WINDOW::REDRAW //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    window::redraw::redraw( window& t ) : target( t )
    {
        scoped_lock< rwlock > slock( target.window_lock, RW_WRITE );
        
        target.pending_redraws++;
    }
    bool window::redraw::execute( task_mask* caller_mask )
    {
        scoped_lock< rwlock > slock( target.window_lock, RW_READ );
        
        target.makeContextCurrent();
        
        target.deleteUnreferencedTextures();
        target.uploadUnuploadedTextures();
        
        if( target.pending_redraws <= 1 )                                       // Only redraw if there are no other pending redraws for that window; this is
                                                                                // safe because the redraw task is high-priority, so the task system will
                                                                                // eventually drill down to the last one.
        {
            if( target.pending_redraws != 1 )                                   // Sanity check
                throw exception( "window::redraw::execute(): Target pending redraws somehow < 1" );
            
            // glLoadIdentity();
            glViewport( 0, 0, target.dimensions[ 0 ], target.dimensions[ 1 ] );
            glLoadIdentity();
            glOrtho( 0.0, target.dimensions[ 0 ], target.dimensions[ 1 ], 0.0, 1.0, -1.0 );
            
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
            // glEnable( GL_DEPTH_TEST );
            glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
            // glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glClear( GL_COLOR_BUFFER_BIT );
            
            glEnable( GL_TEXTURE_2D );
            
            for( int i = 0; i < target.elements.size(); ++i )
                target.elements[ i ] -> draw();
            
            #if defined PLATFORM_XWS_GNUPOSIX
            
            Display* x_display = getXDisplay();
            glXSwapBuffers( x_display, target.platform_window.x_window );
            
            #else
            
            #error "Buffer swapping not implemented on non-X platforms"
            
            #endif
        }
        
        // Attempt to fix initial no-render bug (still happens):
        
        target.window_lock.unlock();
        target.window_lock.lock_write();                                        // Switch the lock to write
        target.pending_redraws--;
        
        return true;
    }
}


