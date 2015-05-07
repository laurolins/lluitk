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
        virtual bool next(Widget* &next_widget) { return false; }
    };
    
    //------------------------------------------------------------------------------
    // WidgetIterator
    //------------------------------------------------------------------------------
    
    struct WidgetIterator {
    public:
        WidgetIterator(BaseWidgetIterator *it):
        it(it)
        {}
        ~WidgetIterator() { delete it; };
        bool next(Widget* &next_widget) { return it->next(next_widget); }
    public:
        BaseWidgetIterator *it { nullptr };
    };
    
    //------------------------------------------------------------------------------
    // Widget
    //------------------------------------------------------------------------------
    
    struct Widget {
    public:
        
        virtual bool           contains(const Point& p) const = 0;
        virtual WidgetIterator children() const { return WidgetIterator(new BaseWidgetIterator()); }
        virtual Widget*        getParent() const = 0;
        virtual void           setParent(Widget* parent) = 0;
        
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

        virtual void sizeHint(const Window &new_window) {} // message that might be used
                                                                 // to redefine boundaries of the
                                                                 // children widget etc.

    };

    //------------------------------------------------------------------------------
    // WidgetWithParent
    //------------------------------------------------------------------------------
    
    struct WidgetWithParent: public Widget {
    public: // on event methods (should be overriden by subclasses)
        virtual Widget*        getParent() const;
        virtual void           setParent(Widget* parent);
    protected:
        Widget* parent { nullptr };
    };

    
    //------------------------------------------------------------------------------
    // Container
    //------------------------------------------------------------------------------
    
    struct Container: public Widget {
    public:
        struct iterator: public BaseWidgetIterator {
        public:
            using container_type = std::vector<Widget*>;
            using base_iter_type = decltype(std::declval<container_type>().cbegin());
        public:
            iterator()=default;
            iterator(const container_type &container);
            bool next(Widget* &next_widget);
        public:
            base_iter_type current;
            base_iter_type end;
        };
    public:
        WidgetIterator children() const;
        
        virtual void addWidget(Widget* widget);
        virtual void removeWidget(Widget* widget);
        
    public:
        std::vector<Widget*> _children;
    };
    
}
