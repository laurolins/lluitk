#include "widget.hh"

namespace lluitk {

    //------------------------------------------------------------------------------
    // WidgetWithParent
    //------------------------------------------------------------------------------
    
    Widget* WidgetWithParent::getParent() const {
        return parent;
    }
    
    void WidgetWithParent::setParent(Widget* parent) {
        this->parent = parent;
    }
    
    //------------------------------------------------------------------------------
    // WidgetContainer::iterator
    //------------------------------------------------------------------------------
    
    Container::iterator::iterator(const container_type &container):
    current(container.cbegin()),
    end(container.cend())
    {}
    
    bool Container::iterator::next(Widget* &widget) {
        if (current != end) {
            widget = *current;
            ++current;
            return true;
        }
        else {
            return false;
        }
    }
    
    //------------------------------------------------------------------------------
    // WidgetContainer
    //------------------------------------------------------------------------------
    
    WidgetIterator Container::children() const {
        return WidgetIterator(new iterator(_children));
    }
    
    void Container::addWidget(Widget* widget) {
        widget->setParent(this);
        _children.push_back(widget);
    }
    
    void Container::removeWidget(Widget* widget) {
        auto it = std::find(_children.begin(), _children.end(), widget);
        if (it != _children.end()) {
            _children.erase(it);
        }
        else {
            throw std::runtime_error("widget is not in container");
        }
    }
    
}
