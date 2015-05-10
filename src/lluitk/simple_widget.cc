#include "simple_widget.hh"

namespace lluitk {
    
    //------------------------------------------------------------------------------
    // WidgetWithParent
    //------------------------------------------------------------------------------
    
    Widget* SimpleWidget::parent() const {
        return _parent;
    }
    
    void SimpleWidget::parent(Widget* parent) {
        _parent = parent;
    }
    
    WidgetStyle& SimpleWidget::style() {
        if (!_style) {
            _style.reset(new WidgetStyle());
        }
        return *_style.get();
    }
    
    SimpleWidget& SimpleWidget::style(const WidgetStyle& style) {
        if (!_style) {
            _style.reset(new WidgetStyle(style));
        }
        auto &s = *_style.get();
        s = style;
        return *this;
    }
    
    const WidgetStyle& SimpleWidget::style() const {
        return *_style.get();
    }
    
    bool SimpleWidget::styleFlag() const {
        return _style.get() != nullptr;
    }
    
    WidgetStyle SimpleWidget::inheritedStyle() const {
        auto result = WidgetStyle::defaultStyle();
        auto w = this;
        while (w) {
            if (w->styleFlag())
                result = result + w->style();
            auto p = w->parent();
            if (p)
                w = p->as<SimpleWidget>();
            else
                w = nullptr;
        }
        return result;
    }

}