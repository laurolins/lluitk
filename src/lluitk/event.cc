#include "event.hh"

#include <sstream>
#include <iostream>
#include <string>

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
        
        //------------------------------------------------------------------------------
        // KeyRelease
        //------------------------------------------------------------------------------

        std::ostream& write(std::ostream& os, const EventType &t) {
            os << "e:" << (int) t << ";";
            return os;
        }
        
        std::ostream& write(std::ostream& os, const MouseButton &b) {
            os << "mb:" << (int) b << ";";
            return os;
        }

        std::ostream& write(std::ostream& os, const KeyCode &code) {
            os << "k:" << (int) code << ";";
            return os;
        }
        
        std::ostream& write(std::ostream& os, const Modifiers &m) {
            int modifier =
            (m.alt      ? 1 : 0) +
            ((m.control ? 1 : 0) << 1) +
            ((m.shift   ? 1 : 0) << 2) +
            ((m.super   ? 1 : 0) << 3);
            os << "mod:" << (int) modifier << ";";
            return os;
        }

        std::ostream& write(std::ostream& os, const Point &p) {
            os << "p:" << p.x() << "," << p.y() << ";";
            return os;
        }

        EventType read_EventType(std::istream& is) {
            std::string lbl;
            std::getline(is,lbl,':');
            if (lbl.compare("e") != 0)
                throw std::runtime_error("ooops");
            std::getline(is,lbl,';');
            return (EventType) std::stoi(lbl);
        }

        KeyCode read_KeyCode(std::istream& is) {
            std::string lbl;
            std::getline(is,lbl,':');
            if (lbl.compare("k") != 0)
                throw std::runtime_error("ooops");
            std::getline(is,lbl,';');
            return (KeyCode) std::stoi(lbl);
        }

        Point read_Point(std::istream& is) {
            std::string lbl;
            std::getline(is,lbl,':');
            if (lbl.compare("p") != 0)
                throw std::runtime_error("ooops");
            std::getline(is,lbl,',');
            double x = std::stod(lbl);
            std::getline(is,lbl,';');
            double y = std::stod(lbl);
            return Point {x,y};
        }

        MouseButton read_MouseButton(std::istream& is) {
            std::string lbl;
            std::getline(is,lbl,':');
            if (lbl.compare("mb") != 0)
                throw std::runtime_error("ooops");
            std::getline(is,lbl,';');
            return (MouseButton) std::stoi(lbl);
        }

        Modifiers read_Modifiers(std::istream& is) {
            std::string lbl;
            std::getline(is,lbl,':');
            if (lbl.compare("mod") != 0)
                throw std::runtime_error("ooops");
            std::getline(is,lbl,';');
            
            auto modifiers = std::stoi(lbl);
            
            bool alt     = modifiers & 0x1;
            bool control = modifiers & 0x2;
            bool shift   = modifiers & 0x4;
            bool super   = modifiers & 0x8;

            return Modifiers {
                shift,
                control,
                alt,
                super
            };
        }

        //------------------------------------------------------------------------------
        // KeyRelease
        //------------------------------------------------------------------------------
        
        void writeEvent(std::ostream& os, const Event& e) {
            auto etype = e.getType();
            write(os, etype);
            if (etype == EVENT_MOUSE_PRESS) {
                auto &ee = e.asMousePress();
                write(os,ee.button);
                write(os,ee.modifiers);
            }
            else if (etype == EVENT_MOUSE_RELEASE) {
                auto &ee = e.asMouseRelease();
                write(os,ee.button);
                write(os,ee.modifiers);
            }
            else if (etype == EVENT_MOUSE_MOVE) {
                auto &ee = e.asMouseMove();
                write(os,ee.position);
            }
            else if (etype == EVENT_MOUSE_WHEEL) {
                auto &ee = e.asMouseWheel();
                write(os,ee.position);
                write(os,ee.modifiers);
            }
            else if (etype == EVENT_KEY_PRESS) {
                auto &ee = e.asKeyPress();
                write(os,ee.key);
                write(os,ee.modifiers);
            }
            else if (etype == EVENT_KEY_RELEASE) {
                auto &ee = e.asKeyRelease();
                write(os,ee.key);
                write(os,ee.modifiers);
            }
            else {
                // pass
            }
            
        }
        
        std::unique_ptr<Event> readEvent(std::istream& is) {
            auto etype = read_EventType(is);
            if (etype == EVENT_MOUSE_PRESS) {
                return std::unique_ptr<Event> {
                    new MousePress { read_MouseButton(is), read_Modifiers(is) }
                };
            }
            else if (etype == EVENT_MOUSE_RELEASE) {
                return std::unique_ptr<Event> {
                    new MouseRelease { read_MouseButton(is), read_Modifiers(is) }
                };
            }
            else if (etype == EVENT_MOUSE_MOVE) {
                return std::unique_ptr<Event> {
                    new MouseMove { read_Point(is) }
                };
            }
            else if (etype == EVENT_MOUSE_WHEEL) {
                return std::unique_ptr<Event> {
                    new MouseWheel { read_Point(is), read_Modifiers(is) }
                };
            }
            else if (etype == EVENT_KEY_PRESS) {
                return std::unique_ptr<Event> {
                    new KeyPress { read_KeyCode(is), read_Modifiers(is) }
                };
            }
            else if (etype == EVENT_KEY_RELEASE) {
                return std::unique_ptr<Event> {
                    new KeyRelease { read_KeyCode(is), read_Modifiers(is) }
                };
            }
            else {
                // pass
            }
            return std::unique_ptr<Event>();
        }
    }
}