#include "event.hh"

namespace lluitk {
    
    //------------------------------------------------------------------------------
    // BEGIN NAMESPACE: event
    //------------------------------------------------------------------------------
    
    namespace event {
        
        
        
        //-------------------------------------------------------------------------------
        // free function
        //-------------------------------------------------------------------------------
        
        bool isprint(KeyCode code) {
            return KEY_SPACE <= code && code <= KEY_GRAVE_ACCENT;
        }
        
        bool ismodifier(KeyCode code) {
            return KEY_LEFT_SHIFT <= code && code <= KEY_RIGHT_SUPER;
        }

        bool isalpha(KeyCode code) {
            return KEY_A <= code && code <= KEY_Z;
        }

        bool isnum(KeyCode code) {
            return KEY_0 <= code && code <= KEY_9;
        }
        
        char ascii(const KeyCode& code, const Modifiers& modifiers) {
            if (!isprint(code)) {
                return (char) code;
            }
            else if (isalpha(code)) {
                return modifiers.shift ? (char) code : std::tolower((char) code);
            }
            else if (!modifiers.shift) {
                return (char) code;
            }
            else {
                switch (code) {
                    case KEY_0:
                        return ')';
                    case KEY_1:
                        return '!';
                    case KEY_2:
                        return '@';
                    case KEY_3:
                        return '#';
                    case KEY_4:
                        return '$';
                    case KEY_5:
                        return '%';
                    case KEY_6:
                        return '^';
                    case KEY_7:
                        return '&';
                    case KEY_8:
                        return '*';
                    case KEY_9:
                        return '(';
                    case KEY_PERIOD:
                        return '>';
                    case KEY_COMMA:
                        return '<';
                    case KEY_SLASH:
                        return '?';
                    case KEY_SEMICOLON:
                        return ':';
                    case KEY_GRAVE_ACCENT:
                        return '~';
                    case KEY_LEFT_BRACKET:
                        return '{';
                    case KEY_RIGHT_BRACKET:
                        return '}';
                    case KEY_EQUAL:
                        return '+';
                    case KEY_MINUS:
                        return '_';
                    case KEY_BACKSLASH:
                        return '|';
                    case KEY_APOSTROPHE:
                        return '\"';
                    default:
                        return (char) code;
                        
                }
                
            }
        }

        
        

        //------------------------------------------------------------------------------
        // MouseMove
        //------------------------------------------------------------------------------
        
        MouseMove::MouseMove(const Point &pos):
        Event(EVENT_MOUSE_MOVE), position(pos)
        {}
        
        
        //------------------------------------------------------------------------------
        // MouseWheel
        //------------------------------------------------------------------------------
        
        MouseWheel::MouseWheel(const Point &pos, const Modifiers &modifiers):
        Event(EVENT_MOUSE_WHEEL), position(pos), modifiers(modifiers)
        {}
        
        //------------------------------------------------------------------------------
        // MousePress
        //------------------------------------------------------------------------------
        
        MousePress::MousePress(MouseButton button, const Modifiers &modifiers):
        Event(EVENT_MOUSE_PRESS), button(button), modifiers(modifiers)
        {}
        
        //------------------------------------------------------------------------------
        // MouseRelease
        //------------------------------------------------------------------------------
        
        MouseRelease::MouseRelease(MouseButton button, const Modifiers &modifiers):
        Event(EVENT_MOUSE_RELEASE), button(button), modifiers(modifiers)
        {}
        
        //------------------------------------------------------------------------------
        // WindowResize
        //------------------------------------------------------------------------------
        
        WindowResize::WindowResize(const Size& new_size):
        Event(EVENT_WINDOW_RESIZE),
        new_size(new_size)
        {}
        
        //------------------------------------------------------------------------------
        // KeyPress
        //------------------------------------------------------------------------------
        
        KeyPress::KeyPress():
        Event(EVENT_KEY_PRESS)
        {}
        
        KeyPress::KeyPress(KeyCode key, const Modifiers &modifiers):
        Event(EVENT_KEY_PRESS),
        key{key},
        modifiers{modifiers}
        {}
        
        //------------------------------------------------------------------------------
        // KeyRelease
        //------------------------------------------------------------------------------
        
        KeyRelease::KeyRelease():
        Event(EVENT_KEY_RELEASE)
        {}
        
        KeyRelease::KeyRelease(KeyCode key, const Modifiers &modifiers):
        Event(EVENT_KEY_RELEASE),
        key{key},
        modifiers{modifiers}
        {}
        
    }
    

    
}