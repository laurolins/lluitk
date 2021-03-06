#pragma once

#include <functional>

#include "llsg/llsg.hh"
#include "simple_widget.hh"
#include "canvas.hh"

namespace lluitk {

    //-----------------------------------------------------
    // TextEdit
    //-----------------------------------------------------
    
    /*! \brief simple text edit
     *
     * The textedit visuals consists of a background rectangle (roundit?) which is
     * painted in bgcolor and text painted in fgcolor. There also a cursor that
     * which (if properly signaled) should blink between the bgcolor and fgcolor.
     *
     * The text is rendered in a certain typeface and fontsize. Note that all text
     * might be horribly aligned. The only alignment parameter is a simple offset
     * from the bottom left corner that should be added to initial text rendering
     * reference point.
     *
     */
    
    struct TextEdit: public SimpleWidget {
    public:
        
        using TriggerFunction = std::function<void(const std::string&)>;
        
    public:
        TextEdit() = default;
        void render(); // assuming opengl context in pixel
    private:
        void prepareCanvas();
    public:
        bool contains(const Point& p) const;
        void sizeHint(const Window &window);
        void onKeyPress(const App &app);
        void onMouseMove(const App &app);
        
        bool acceptsKeyEvents() const { return true; }
        void setKeyFocus(bool focused);


        const Vec2& offset() const;
        TextEdit&   offset(const Vec2& o);
        
        TextEdit& triggerFunction(TriggerFunction t); // set trigger function
        
        void blink(int parity); // message indicating to update cursor
                                // if it is indeed in focus show transition to
                                // another state
        
    public:
        std::vector<char> _text;
        std::size_t       _cursor { 0 };
        Window            _window;
        Canvas            _canvas;
        int               _parity { 0 };
        TriggerFunction   _trigger;
        llsg::Vec2        _offset { 5, 5 };
        bool              _is_focused { false };
    };

}
