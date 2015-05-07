#include "event.hh"

namespace lluitk {
    
    //------------------------------------------------------------------------------
    // BEGIN NAMESPACE: event
    //------------------------------------------------------------------------------
    
    namespace event {
        
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