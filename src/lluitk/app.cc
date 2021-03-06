#include "app.hh"

#include "widget.hh"

#include <chrono>

namespace lluitk {

    static Microseconds now() {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    
    //------------------------------------------------------------------------------
    // App
    //------------------------------------------------------------------------------
    
    App::App() {
        _start_time = now();
    }

    
    void App::setMainWidget(Widget *w) {
        main_widget = w;

        WidgetTreeIterator iter(*w);
        Widget* ww;
        while ( (ww = iter.next() ) ) {
            if (ww->acceptsKeyEvents()) {
                setKeyFocus(ww);
                break;
            }
        }
    }

    void App::startEventProcessing() const {
        event_done = false;
    }

    void App::finishEventProcessing() const {
        event_done = true;
    }

    void App::lock(Widget *w) const {
        _locked_widget = w; // _locked_widget is mutable...
    }

    void App::setKeyFocus(Widget *w) {
        if (_key_focus_widget) {
            _key_focus_widget->setKeyFocus(false);
        }
        _key_focus_widget = w;
        if (_key_focus_widget) {
            _key_focus_widget->setKeyFocus(true);
        }
    }
    
    void App::processEvent(const event::Event& event) {

        const event::Event* e = &event;
        
        if (!main_widget)
            return;

        current_event_info            = last_event_info;
        current_event_info.event_type = e->getType();
        current_event_info.timestamp(now() - _start_time);
        
        // window resize is special
        if (e->getType() == event::EVENT_WINDOW_RESIZE) {
            auto ee = dynamic_cast<const event::WindowResize*>(e);
            if (main_widget)
                main_widget->sizeHint(Window { Point {0,0}, ee->new_size } ); // main window gets the new size
            last_event_info = current_event_info;
            return;
        }

        if (e->getType() == event::EVENT_MOUSE_MOVE) {
            auto ee = dynamic_cast<const event::MouseMove*>(e);
            auto p = ee->position;
            current_event_info.mouse_position    = p;
        }
        else if (e->getType() == event::EVENT_MOUSE_WHEEL) {
            auto ee = dynamic_cast<const event::MouseWheel*>(e);
            current_event_info.modifiers         = ee->modifiers;
            current_event_info.mouse_wheel_delta = ee->position;
        }
        else if (e->getType() == event::EVENT_MOUSE_PRESS) {
            auto ee = dynamic_cast<const event::MousePress*>(e);
            current_event_info.modifiers = ee->modifiers;
            current_event_info.button    = ee->button;
        }
        else if (e->getType() == event::EVENT_MOUSE_RELEASE) {
            auto ee = dynamic_cast<const event::MouseRelease*>(e);
            current_event_info.modifiers = ee->modifiers;
            current_event_info.button    = ee->button;
        }
        else if (e->getType() == event::EVENT_MOUSE_WHEEL) {
            auto ee = dynamic_cast<const event::MouseWheel*>(e);
            current_event_info.modifiers = ee->modifiers;
            current_event_info.mouse_wheel_delta = ee->position;
        }
        else if (e->getType() == event::EVENT_KEY_PRESS) {
            auto ee = dynamic_cast<const event::KeyPress*>(e);
            current_event_info.key_code  = ee->key;
            current_event_info.modifiers = ee->modifiers;
        }
        else if (e->getType() == event::EVENT_KEY_RELEASE) {
            auto ee = dynamic_cast<const event::KeyRelease*>(e);
            current_event_info.key_code  = ee->key;
            current_event_info.modifiers = ee->modifiers;
        }

        
        auto &app = *this;
        auto send_message = [&](Widget &active_widget, event::EventType event_type) {
            switch(event_type) {
            case event::EVENT_MOUSE_MOVE:
            active_widget.onMouseMove(app);
            break;
            case event::EVENT_KEY_RELEASE:
            active_widget.onKeyRelease(app);
            break;
            case event::EVENT_KEY_PRESS:
            active_widget.onKeyPress(app);
            break;
            case event::EVENT_MOUSE_WHEEL:
            active_widget.onMouseWheel(app);
            break;
            case event::EVENT_MOUSE_PRESS:
            active_widget.onMousePress(app);
            break;
            case event::EVENT_MOUSE_RELEASE:
            active_widget.onMouseRelease(app);
            break;
            default:
            break;
            }
        };
        
        if (e->getType() == event::EVENT_KEY_PRESS || e->getType() == event::EVENT_KEY_RELEASE) {
            if (_key_focus_widget) {
                send_message(*_key_focus_widget, e->getType());
            }
        }
        else {
            this->startEventProcessing();
            
            Widget *active_widget = nullptr;
            
            auto p = current_event_info.mouse_position;
            
            if (_locked_widget) {
                active_widget = _locked_widget;
            }
            else if (main_widget->contains(p)) {
                active_widget = main_widget;
            }
            
            if (active_widget) {
                bool update = true;
                while (update) {
                    
                    send_message(*active_widget, e->getType());
                    
                    if (event_done)
                        break;
                    
                    // update
                    update = false;
                    Widget *child = nullptr;
                    auto it = active_widget->reverse_children(); // needs to process in reverse order here
                    while ((child = it.next())) {
                        if (child->contains(p)) {
                            active_widget = child;
                            update = true;
                            break;
                        }
                    }
                }
            }

            
            //
            // change key focused widget
            //
            if (e->getType() == event::EVENT_MOUSE_PRESS) {
                if (active_widget && active_widget->acceptsKeyEvents()) {
                    if (_key_focus_widget) {
                        _key_focus_widget->setKeyFocus(false);
                    }
                    _key_focus_widget = active_widget;
                    _key_focus_widget->setKeyFocus(true);
                }
            }

        }
        
        last_event_info = current_event_info;

    }

}

