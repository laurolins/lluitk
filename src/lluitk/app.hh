#pragma once

#include "event.hh"

namespace lluitk {

    //------------------------------------------------------------------------------
    // Forward Decl.
    //------------------------------------------------------------------------------
    
    struct Widget;

    //------------------------------------------------------------------------------
    // EventInfo
    //------------------------------------------------------------------------------
    
    struct EventInfo {
        EventInfo() = default;
        event::EventType event_type;
        Point  mouse_press_position;
        Point  mouse_release_position;
        Point  mouse_position;
        Point  mouse_wheel_delta;
        Widget      *mouse_over_widget { nullptr };
        event::Modifiers    modifiers;
        event::MouseButton  button;
        event::KeyCode      key_code;
    };

    //------------------------------------------------------------------------------
    // App
    //------------------------------------------------------------------------------
    
    struct App {
        void setMainWidget(Widget *w);
        void processEvent(const event::Event &event);
        
        void startEventProcessing() const;
        void finishEventProcessing() const;

        void lock(Widget *w=nullptr) const; // without argument or null it unlocks
        
        void setKeyFocus(Widget *w);
        
    public:
        // keep a key_focus, hover, drag-n-drop model
        Widget* main_widget { nullptr };
        
        mutable Widget* _locked_widget { nullptr };
        mutable Widget* _key_focus_widget { nullptr };
        mutable bool event_done { false };

        EventInfo  last_event_info;
        EventInfo  current_event_info;
    };

}
