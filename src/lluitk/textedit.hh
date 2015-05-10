#pragma once

#include "llsg.hh"
#include "widget.hh"
#include "canvas.hh"

namespace lluitk {

    //-----------------------------------------------------
    // TextEdit
    //-----------------------------------------------------
    
    struct TextEdit: public WidgetWithParent {
    public:
        TextEdit() = default;
        void render(); // assuming opengl context in pixel
    private:
        void prepareCanvas();
    public:
        bool contains(const Point& p) const;
        void sizeHint(const Window &window);
        void onKeyPress(const App &app);
        
        void blink(int parity); // message indicating to update cursor
                                // if it is indeed in focus show transition to
                                // another state
        
    public:
        std::vector<char> _text;
        std::size_t       _cursor { 0 };
        Window            _window;
        Canvas            _canvas;
        int               _parity { 0 };
    };

}
