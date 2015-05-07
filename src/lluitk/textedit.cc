#include "textedit.hh"

#include <algorithm>

#include "llsg_opengl.hh"

#include "d3cpp.hh"
#include "app.hh"

namespace lluitk {
    
    //--------------------------------------------------------------------------
    // Canvas
    //--------------------------------------------------------------------------
    
    Canvas& Canvas::markDirty(bool flag)
    {
        this->dirty = flag;
        return *this;
    }
    
    //--------------------------------------------------------------------------
    // TextEdit
    //--------------------------------------------------------------------------
    
    bool TextEdit::contains(const Point& p) const {
        return _window.contains(p);
    }

    void TextEdit::sizeHint(const Window &window) {
        _window = window;
        _canvas.markDirty();
    }

    void TextEdit::render() {
        if (_canvas.dirty) {
            prepareCanvas();
        }
        
        auto &renderer = llsg::opengl::getRenderer();
        
        // llsg::print(std::cerr, canvas.root);
        renderer.render(_canvas.root, llsg::Transform(), _window);
    }
    
    void TextEdit::prepareCanvas() {
        
        using document_type = d3cpp::Document<llsg::Element>;
        
        document_type document(&_canvas.root);

        auto sel = document
        .selectAll(llsg::isText, llsg::iter(1,1))
        .data( std::vector<std::string> { std::string(_text.begin(),_text.end()) });
        
        sel
        .exit()
        .remove([](llsg::Element* e) { e->remove(); });
        
        sel
        .enter()
        .append([](llsg::Element* parent, const std::string& s) { return &parent->asGroup().text(); });

        sel
        .call([&](llsg::Element* e, const std::string& s) {
            auto &t = e->asText();
            t
            .str(s)
            .pos(_window.min() + llsg::Vec2 { 5, 5});
        });
    }

    void TextEdit::onKeyPress(const App &app) {
        auto code = app.current_event_info.key_code;
        if ( (event::KEY_A <= code && code <= event::KEY_Z) || code == event::KEY_SPACE) {
            _text.insert(_text.begin() + _cursor, (char) code);
            ++_cursor;
        }
        else if (code == event::KEY_BACKSPACE) {
            _text.erase(_text.begin() + _cursor - 1);
            --_cursor;
        }
        _canvas.markDirty();
    }
    
}
