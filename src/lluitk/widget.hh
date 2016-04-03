#pragma once

#include <vector>

#include "geom.hh"
#include "event.hh"

namespace lluitk {

    //------------------------------------------------------------------------------
    // Forward Decl.
    //------------------------------------------------------------------------------
    
    struct App;
    struct Widget;

    //------------------------------------------------------------------------------
    // BaseWidgetIterator
    //------------------------------------------------------------------------------
    
    struct BaseWidgetIterator {
        virtual ~BaseWidgetIterator() {}
        virtual Widget* next() { return nullptr; }
    };
    
    //------------------------------------------------------------------------------
    // WidgetIterator
    //------------------------------------------------------------------------------
    
    struct WidgetIterator {
    public:
        WidgetIterator() = default;
        WidgetIterator(BaseWidgetIterator *it): it(it) {}
        ~WidgetIterator() { delete it; };
        Widget* next() { return (it) ? it->next() : nullptr; }
    public:
        BaseWidgetIterator *it { nullptr };
    };
    
    //------------------------------------------------------------------------------
    // Widget
    //------------------------------------------------------------------------------
    
    struct Widget {
    public:
        
        virtual bool           contains(const Point& p) const { return false; }
        
        // bottom-up (rendering order)
        virtual WidgetIterator children() const { return WidgetIterator(); }
        
        // top-down (event processing priority)
        virtual WidgetIterator reverse_children() const { return WidgetIterator(); }
        
        virtual Widget*        parent() const { return nullptr; }
        virtual void           parent(Widget* parent) { };
        
        template <typename T>
        T* as() {
            return dynamic_cast<T*>(this);
        }
        
    public: // on event methods (should be overriden by subclasses)
        
        virtual void render() {}
        virtual void pre_render() {}

        virtual void onMousePress(const App &app) {}
        virtual void onMouseRelease(const App &app) {}

        virtual void onKeyPress(const App &app) {}
        virtual void onKeyRelease(const App &app) {}

        virtual void onMouseEnter(const App &app) {}
        virtual void onMouseLeave(const App &app) {}
        
        virtual void onMouseMove(const App &app) {}
        virtual void onMouseWheel(const App &app) {}

        virtual bool acceptsKeyEvents() const { return false; }
        virtual void setKeyFocus(bool focused) { }
        
        virtual void blink(int parity) {};
        
        virtual void sizeHint(const Window &new_window) {} // message that might be used
                                                                 // to redefine boundaries of the
                                                                 // children widget etc.

    };

    //----------------------------------------------------------------------------
    // WidgetTreeIterator
    //----------------------------------------------------------------------------
    
    struct WidgetTreeIterator {
        WidgetTreeIterator(Widget& root);
        Widget *next();
        std::vector<Widget*> stack;
    };
    


    
}
