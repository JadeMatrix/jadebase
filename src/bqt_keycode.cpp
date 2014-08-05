/* 
 * bqt_keycode.cpp
 * 
 * Implements conversion utilities from bqt_keycode.hpp
 * 
 * Also implements getKeyCommandString() from bqt_windowevent.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_keycode.hpp"

#include "bqt_exception.hpp"
#include "bqt_windowevent.hpp"

/* INTERNAL GLOBALS ***********************************************************//******************************************************************************/

namespace
{
    
}

/******************************************************************************//******************************************************************************/

namespace bqt
{
    keycode convertPlatformKeycode( bqt_platform_keycode_t k )
    {
        #if defined PLATFORM_XWS_GNUPOSIX
        
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
        
        return KEY_INVALID;
        
        #else
        
        #error Keycode conversion not implemented on non-X platforms
        
        #endif
    }
    
    const char* getKeycodeString( keycode k )
    {
        switch( k )
        {
            case KEY_INVALID:
                // throw exception( "getKeycodeString(): Unsupported keycode value" );
                return "[invalid key]";
            
            case KEY_Delete:
                return "Delete/Return";
            case KEY_Tab:
                return "Tab";
            case KEY_Enter:
                return "Enter/Return";
            case KEY_Escape:
                return "Esc";

            case KEY_Left:
                return "Left";
            case KEY_Right:
                return "Right";
            case KEY_Up:
                return "Up";
            case KEY_Down:
                return "Down";
            case KEY_Home:
                return "Home";
            case KEY_End:
                return "End";
            case KEY_PageUp:
                return "PageUp";
            case KEY_PageDown:
                return "PageDown";
            case KEY_Next:
                return "Next";
            case KEY_Previous:
                return "Previous";

            case KEY_F1:
                return "F1";
            case KEY_F2:
                return "F2";
            case KEY_F3:
                return "F3";
            case KEY_F4:
                return "F4";
            case KEY_F5:
                return "F5";
            case KEY_F6:
                return "F6";
            case KEY_F7:
                return "F7";
            case KEY_F8:
                return "F8";
            case KEY_F9:
                return "F9";
            case KEY_F10:
                return "F10";
            case KEY_F11:
                return "F11";
            case KEY_F12:
                return "F12";
            case KEY_F13:
                return "F13";
            case KEY_F14:
                return "F14";
            case KEY_F15:
                return "F15";
            case KEY_F16:
                return "F16";
            case KEY_F17:
                return "F17";
            case KEY_F18:
                return "F18";
            case KEY_F19:
                return "F19";
            case KEY_F20:
                return "F20";
            case KEY_F21:
                return "F21";
            case KEY_F22:
                return "F22";
            case KEY_F23:
                return "F23";
            case KEY_F24:
                return "F24";
            case KEY_F25:
                return "F25";
            case KEY_F26:
                return "F26";
            case KEY_F27:
                return "F27";
            case KEY_F28:
                return "F28";
            case KEY_F29:
                return "F29";
            case KEY_F30:
                return "F30";
            case KEY_F31:
                return "F31";
            case KEY_F32:
                return "F32";
            case KEY_F33:
                return "F33";
            case KEY_F34:
                return "F34";
            case KEY_F35:
                return "F35";

            case KEY_BracketLeft:
                return "[";
            case KEY_BracketRight:
                return "]";
            case KEY_Backslash:
                return "\\";
            case KEY_Semicolon:
                return ";";
            case KEY_Apostrophe:
                return "'";
            case KEY_Comma:
                return ",";
            case KEY_Period:
                return ".";
            case KEY_Slash:
                return "/";
            case KEY_Grave:
                return "`";

            case KEY_0:
                return "0";
            case KEY_1:
                return "1";
            case KEY_2:
                return "2";
            case KEY_3:
                return "3";
            case KEY_4:
                return "4";
            case KEY_5:
                return "5";
            case KEY_6:
                return "6";
            case KEY_7:
                return "7";
            case KEY_8:
                return "8";
            case KEY_9:
                return "9";

            case KEY_Minus:
                return "-";
            case KEY_Equal:
                return "=";

            case KEY_A:
                return "A";
            case KEY_B:
                return "B";
            case KEY_C:
                return "C";
            case KEY_D:
                return "D";
            case KEY_E:
                return "E";
            case KEY_F:
                return "F";
            case KEY_G:
                return "G";
            case KEY_H:
                return "H";
            case KEY_I:
                return "I";
            case KEY_J:
                return "J";
            case KEY_K:
                return "K";
            case KEY_L:
                return "L";
            case KEY_M:
                return "M";
            case KEY_N:
                return "N";
            case KEY_O:
                return "O";
            case KEY_P:
                return "P";
            case KEY_Q:
                return "Q";
            case KEY_R:
                return "R";
            case KEY_S:
                return "S";
            case KEY_T:
                return "T";
            case KEY_U:
                return "U";
            case KEY_V:
                return "V";
            case KEY_W:
                return "W";
            case KEY_X:
                return "X";
            case KEY_Y:
                return "Y";
            case KEY_Z:
                return "Z";

            case KEY_Tilde:
                return "~";
            case KEY_ExclamationPoint:
                return "!";
            case KEY_At:
                return "@";
            case KEY_Hash:
                return "#";
            case KEY_Dollar:
                return "$";
            case KEY_Percent:
                return "%";
            case KEY_Caret:
                return "^";
            case KEY_Ampersand:
                return "&";
            case KEY_Asterisk:
                return "*";
            case KEY_ParenLeft:
                return "(";
            case KEY_ParenRight:
                return ")";
            case KEY_Underscore:
                return "_";
            case KEY_Plus:
                return "+";

            case KEY_BraceLeft:
                return "{";
            case KEY_BraceRight:
                return "}";
            case KEY_Bar:
                return "|";
            case KEY_Colon:
                return ":";
            case KEY_Quote:
                return "\"";
            case KEY_LessThan:
                return "<";
            case KEY_GreaterThan:
                return ">";
            case KEY_QuestionMark:
                return "?";
            
            default:
                throw exception( "getKeycodeString(): Unknown keycode value" );
        }
    }
    
    std::string getKeyCommandString( key_command& k )
    {
        std::string kc_string;
        
        if( k.super )
            #if defined PLATFORM_MACOSX
            kc_string += "Cmd-";
            #elif defined PLATFORM_WINDOWS
            kc_string += "Win-";
            #else
            kc_string += "Super-";
            #endif
        if( k.ctrl )
            kc_string += "Ctrl-";
        if( k.alt )
            #ifdef PLATFORM_MACOSX
            kc_string += "Opt-";
            #else
            kc_string += "Alt-";
            #endif
        if( k.shift )
            kc_string += "Shift-";
        
        kc_string += getKeycodeString( k.key );
        
        return kc_string;
    }
}


