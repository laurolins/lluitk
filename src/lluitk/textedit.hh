#pragma once

#include "llsg.hh"
#include "widget.hh"

namespace lluitk {

    //------------------------------------------------------------------------------
    // Canvas
    //------------------------------------------------------------------------------
    
    struct Canvas {
        Canvas() = default;
        
        Canvas& markDirty(bool flag=true);
        bool          dirty   { true };

        llsg::Group   root;
    };

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
    public:
    
        std::vector<char> _text;
        std::size_t       _cursor { 0 };

        Window            _window;
       
        Canvas            _canvas;
    
    };

}
