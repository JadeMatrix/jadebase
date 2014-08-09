#ifndef BQT_KEYCODE_HPP
#define BQT_KEYCODE_HPP

/* 
 * bqt_keycode.hpp
 * 
 * Definitions for the internal keycode type & utilities for conversion from the
 * platform keycode type.
 * 
 * Conversion notes:
 *   - Macro keys (such as Shift or CTRL) are ignored as they should be
 *     contained in each X event's state field.
 *   - BQTDraw performs the same action for Delete/Backspace and Enter/Return,
 *     so these keys are treated as the same.
 *   - BQTDraw only accepts Western-type keyboard keys for now.
 *     TODO: Fix this as part of translations
 *   - BQTDraw does not handle the Help key, as there might be a system-specific
 *     action this triggers; help menus are found using in-GUI tooltips.
 *   - BQTDraw has no functionality that depends on any of the *lock keys, so
 *     they are ignored
 *   - Keypad & regular keys that have the same meaning are treated the same.
 *     This includes Slash & KP Divide, as while some keyboards use the division
 *     symbol for KP Divide, Slash is universally recognized as a division
 *     symbol.
 *   - There are 35 F keys since that's as many as XLib supports.  Keyboards
 *     with left & right sets (as supported by XLib) are converted to their F
 *     equivalents, as well as other function keys such as Logitech's G-series'
 *     G keys.
 * 
 */

/* INCLUDES *******************************************************************//******************************************************************************/

#include "bqt_platform.h"

/******************************************************************************//******************************************************************************/

namespace bqt
{
    enum keycode
    {
        KEY_INVALID,
        
        KEY_Delete,
        KEY_Tab,
        KEY_Enter,
        KEY_Escape,
        
        KEY_Left,
        KEY_Right,
        KEY_Up,
        KEY_Down,
        KEY_Home,
        KEY_End,
        KEY_PageUp,
        KEY_PageDown,
        KEY_Next,
        KEY_Previous,
        
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,
        KEY_F13,
        KEY_F14,
        KEY_F15,
        KEY_F16,
        KEY_F17,
        KEY_F18,
        KEY_F19,
        KEY_F20,
        KEY_F21,
        KEY_F22,
        KEY_F23,
        KEY_F24,
        KEY_F25,
        KEY_F26,
        KEY_F27,
        KEY_F28,
        KEY_F29,
        KEY_F30,
        KEY_F31,
        KEY_F32,
        KEY_F33,
        KEY_F34,
        KEY_F35,
        
        KEY_BracketLeft,
        KEY_BracketRight,
        KEY_Backslash,
        KEY_Semicolon,
        KEY_Apostrophe,
        KEY_Comma,
        KEY_Period,
        KEY_Slash,
        KEY_Grave,                                                              // AKA Backquote
        
        KEY_0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        
        KEY_Minus,                                                              // Dash is different, such as generated by Option-Minus on OS X
        KEY_Equal,
        
        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,
        
        KEY_Tilde,
        KEY_ExclamationPoint,
        KEY_At,
        KEY_Hash,
        KEY_Dollar,
        KEY_Percent,
        KEY_Caret,
        KEY_Ampersand,
        KEY_Asterisk,
        KEY_ParenLeft,
        KEY_ParenRight,
        KEY_Underscore,
        KEY_Plus,
        
        KEY_BraceLeft,
        KEY_BraceRight,
        KEY_Bar,
        KEY_Colon,
        KEY_Quote,
        KEY_LessThan,
        KEY_GreaterThan,
        KEY_QuestionMark
        
        // TODO: add support for international keyboards
    };
    
    keycode convertPlatformKeycode( bqt_platform_keycode_t k );
    const char* getKeycodeString( keycode k );
}

/******************************************************************************//******************************************************************************/

#endif

