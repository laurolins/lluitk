#include "widget.hh"

namespace lluitk {
    
    //------------------------------------------------------------------------------
    // WidgetTreeIterator
    //------------------------------------------------------------------------------

    WidgetTreeIterator::WidgetTreeIterator(Widget& root) {
        stack.push_back(&root);
    }

    Widget* WidgetTreeIterator::next() {
        if (stack.empty())
            return nullptr;
        
        auto result = stack.back();
        stack.pop_back();
        
        auto iter = result->children();
        
        std::vector<Widget*> children;
        Widget* child;
        while ((child = iter.next())) {
            children.push_back(child);
        }
        
        for (auto it=children.rbegin();it!=children.rend();++it) {
            stack.push_back(*it);
        }

        return result;
    }

}
