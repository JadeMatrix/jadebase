/* 
 * jb_keycode.cpp
 * 
 * Implements platform-generic utilities from jb_keycode.hpp
 * 
 * Also implements getKeyCommandString() from jb_windowevent.hpp
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "jb_keycode.hpp"

#include "jb_windowevent.hpp"
#include "../utility/jb_exception.hpp"

/******************************************************************************//******************************************************************************/

namespace jade
{
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


