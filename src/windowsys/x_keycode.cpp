/* 
 * x_keycode.cpp
 * 
 * Implements conversion utilities from jb_keycode.hpp under XWS
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_keycode.hpp"

#include <map>

#include "jb_windowevent.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    #if 0
    std::map< jb_platform_keycode_t, jade::keycode > partial_keycode_map = { { XK_Delete, KEY_Delete },
                                                                             { XK_KP_Delete, KEY_Delete },
                                                                             { XK_BackSpace, KEY_Delete },
                                                                             { XK_Tab, KEY_Tab },
                                                                             { XK_KP_Tab, KEY_Tab },
                                                                             // { XK_ISO_Left_Tab, KEY_Tab },
                                                                             { XK_Return, KEY_Enter },
                                                                             { XK_KP_Enter, KEY_Enter },
                                                                             // { XK_ISO_Enter, KEY_Enter },
                                                                             { XK_Escape, KEY_Escape },
                                                                             
                                                                             { XK_Left, KEY_Left },
                                                                             { XK_KP_Left, KEY_Left },
                                                                             { XK_Right, KEY_Right },
                                                                             { XK_KP_Right, KEY_Right },
                                                                             { XK_Up, KEY_UP },
                                                                             { XK_KP_Up, KEY_UP },
                                                                             { XK_Down, KEY_Down },
                                                                             { XK_KP_Down, KEY_Down },
                                                                             { XK_Home, KEY_Home },
                                                                             { XK_KP_Home, KEY_Home },
                                                                             { XK_End, KEY_End },
                                                                             { XK_KP_End, KEY_End },
                                                                             { XK_Page_Up, KEY_PageUp },
                                                                             { XK_KP_Page_Up, KEY_PageUp },
                                                                             { XK_Page_Down, KEY_PageDown },
                                                                             { XK_KP_Page_Down, KEY_PageDown },
                                                                             { XK_Next, KEY_Next },
                                                                             { XK_KP_Next, KEY_Next },
                                                                             { XK_Prior, KEY_Previous },
                                                                             { XK_KP_Prior, KEY_Previous },
                                                                             
                                                                             { XK_bracketleft, KEY_BracketLeft },
                                                                             { XK_bracketright, KEY_BracketRight },
                                                                             { XK_backslash, KEY_Backslash },
                                                                             { XK_semicolon, KEY_Semicolon },
                                                                             { XK_apostrophe, KEY_Apostrophe },
                                                                             { XK_comma, KEY_Comma },
                                                                             { XK_period, KEY_Period },
                                                                             { XK_KP_Decimal, KEY_Period },
                                                                             { XK_slash, KEY_Slash },
                                                                             { XK_KP_Divide, KEY_Slash },
                                                                             { XK_grave, KEY_Grave },
                                                                             { XK_dead_grave, KEY_Grave },
                                                                             
                                                                             { XK_minus, KEY_Minus },
                                                                             { XK_KP_Subtract, KEY_Minus },
                                                                             { XK_equal, KEY_Equal },
                                                                             { XK_KP_Equal, KEY_Equal },
                                                                             
                                                                             { XK_asciitilde, KEY_Tilde },
                                                                             { XK_dead_tilde, KEY_Tilde },
                                                                             { XK_exclam, KEY_ExclamationPoint },
                                                                             { XK_at, KEY_At },
                                                                             { XK_numbersign, KEY_Hash },
                                                                             { XK_dollar, KEY_Dollar },
                                                                             { XK_percent, KEY_Percent },
                                                                             { XK_asciicircum, KEY_Caret },
                                                                             // { XK_caret, KEY_Caret },
                                                                             // { XK_upcaret, KEY_Caret },
                                                                             { XK_ampersand, KEY_Ampersand },
                                                                             { XK_asterisk, KEY_Asterisk },
                                                                             { XK_parenleft, KEY_ParenLeft },
                                                                             { XK_parenright, KEY_ParenRight },
                                                                             { XK_underscore, KEY_Underscore },
                                                                             { XK_plus, KEY_Plus },
                                                                             { XK_KP_Add, KEY_Plus },
                                                                             
                                                                             { XK_braceleft, KEY_BraceLeft },
                                                                             { XK_braceright, KEY_BraceRight },
                                                                             { XK_bar, KEY_Bar },
                                                                             { XK_colon, KEY_Colon },
                                                                             { XK_quotedbl, KEY_Quote },
                                                                             { XK_less, KEY_LessThan },
                                                                             // { XK_KP_leftcaret, KEY_LessThan },
                                                                             { XK_greater, KEY_GreaterThan },
                                                                             // { XK_KP_rightcaret, KEY_GreaterThan },
                                                                             { XK_question, KEY_QuestionMark },
                                                                             
                                                                             { XK_KP_Space, KEY_Space },
                                                                             { XK_space, KEY_Space } };
    #endif
}

/******************************************************************************//******************************************************************************/

namespace jade
{
    keycode convertPlatformKeycode( jb_platform_keycode_t k )
    {
        // We ignore macro keys (Shift, CTRL, etc.) as they should be in each
        // X event's state field
        
        // We don't use a switch here as that would result in a huge jump table;
        // as many of the keys are in series we can optimize those conversions.
        
        // Ranges first, letter keys are more common
        
        if( k >= XK_a && k <= XK_z )
            return ( keycode )( KEY_A + ( k - XK_a ) );
        if( k >= XK_A && k <= XK_Z )
            return ( keycode )( KEY_A + ( k - XK_A ) );
        
        if( k >= XK_F1 && k <= XK_F35 )
            return ( keycode )( KEY_F1 + ( k - XK_F1 ) );
        if( k >= XK_KP_F1 && k <= XK_KP_F4 )
            return ( keycode )( KEY_F1 + ( k - XK_KP_F1 ) );
        
        if( k >= XK_0 && k <= XK_9 )
            return ( keycode )( KEY_0 + ( k - XK_0 ) );
        if( k >= XK_KP_0 && k <= XK_KP_9 )
            return ( keycode )( KEY_0 + ( k - XK_KP_0 ) );
        
        // Now stuff that's harder to optimize
        
        // TODO: Perhaps a tree (such as std::map) would be better at this point?
        
        if( k == XK_Delete || k == XK_KP_Delete || k == XK_BackSpace )
            return KEY_Delete;
        if( k == XK_Tab || k == XK_KP_Tab /* || k == XK_ISO_Left_Tab */ )
            return KEY_Tab;
        if( k == XK_Return || k == XK_KP_Enter /* || k == XK_ISO_Enter */ )
            return KEY_Enter;
        if( k == XK_Escape )
            return KEY_Escape;
        
        if( k == XK_Left || k == XK_KP_Left )
            return KEY_Left;
        if( k == XK_Right || k == XK_KP_Right )
            return KEY_Right;
        if( k == XK_Up || k == XK_KP_Up )
            return KEY_Up;
        if( k == XK_Down || k == XK_KP_Down )
            return KEY_Down;
        if( k == XK_Home || k == XK_KP_Home )
            return KEY_Home;
        if( k == XK_End || k == XK_KP_End )
            return KEY_End;
        if( k == XK_Page_Up || k == XK_KP_Page_Up )
            return KEY_PageUp;
        if( k == XK_Page_Down || k == XK_KP_Page_Down )
            return KEY_PageDown;
        if( k == XK_Next || k == XK_KP_Next )
            return KEY_Next;
        if( k == XK_Prior || k == XK_KP_Prior )
            return KEY_Previous;
        
        if( k == XK_bracketleft )
            return KEY_BracketLeft;
        if( k == XK_bracketright )
            return KEY_BracketRight;
        if( k == XK_backslash )
            return KEY_Backslash;
        if( k == XK_semicolon )
            return KEY_Semicolon;
        if( k == XK_apostrophe )
            return KEY_Apostrophe;
        if( k == XK_comma )
            return KEY_Comma;
        if( k == XK_period || k == XK_KP_Decimal )
            return KEY_Period;
        if( k == XK_slash || k == XK_KP_Divide )
            return KEY_Slash;
        if( k == XK_grave || k == XK_dead_grave )
            return KEY_Grave;
        
        if( k == XK_minus || k == XK_KP_Subtract )
            return KEY_Minus;
        if( k == XK_equal || k == XK_KP_Equal )
            return KEY_Equal;
        
        if( k == XK_asciitilde || k == XK_dead_tilde )
            return KEY_Tilde;
        if( k == XK_exclam )
            return KEY_ExclamationPoint;
        if( k == XK_at )
            return KEY_At;
        if( k == XK_numbersign )
            return KEY_Hash;
        if( k == XK_dollar )
            return KEY_Dollar;
        if( k == XK_percent )
            return KEY_Percent;
        if( k == XK_asciicircum /* || k == XK_caret || k == XK_upcaret */ )
            return KEY_Caret;
        if( k == XK_ampersand )
            return KEY_Ampersand;
        if( k == XK_asterisk )
            return KEY_Asterisk;
        if( k == XK_parenleft )
            return KEY_ParenLeft;
        if( k == XK_parenright )
            return KEY_ParenRight;
        if( k == XK_underscore )
            return KEY_Underscore;
        if( k == XK_plus || k == XK_KP_Add )
            return KEY_Plus;
        
        if( k == XK_braceleft )
            return KEY_BraceLeft;
        if( k == XK_braceright )
            return KEY_BraceRight;
        if( k == XK_bar )
            return KEY_Bar;
        if( k == XK_colon )
            return KEY_Colon;
        if( k == XK_quotedbl )
            return KEY_Quote;
        if( k == XK_less /* || k == XK_KP_leftcaret */ )
            return KEY_LessThan;
        if( k == XK_greater /* || k == XK_KP_rightcaret */ )
            return KEY_GreaterThan;
        if( k == XK_question )
            return KEY_QuestionMark;
        
        if( k == XK_KP_Space || k == XK_space )
            return KEY_Space;
        
        return KEY_INVALID;
    }
}


