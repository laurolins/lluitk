#pragma once

#include <memory>

#include "widget.hh"
#include "style.hh"

namespace  lluitk {

    //------------------------------------------------------------------------------
    // WidgetWithParent
    //------------------------------------------------------------------------------
    
    struct SimpleWidget: public Widget {
    public: // on event methods (should be overriden by subclasses)
        
        virtual Widget*        parent() const;
        virtual void           parent(Widget* parent);
        
        WidgetStyle&           style();
        const WidgetStyle&     style() const;
        SimpleWidget&          style(const WidgetStyle& s);
        bool                   styleFlag() const;

        WidgetStyle            inheritedStyle() const;
    
    protected:
        Widget*                      _parent { nullptr };
        std::unique_ptr<WidgetStyle> _style;
    };

}