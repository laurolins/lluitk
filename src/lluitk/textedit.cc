#include "textedit.hh"

#include <algorithm>

#include "llsg_opengl.hh"

#include "d3cpp.hh"
#include "app.hh"
#include "transition.hh"

namespace lluitk {

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
        
        _canvas.root.identity().translate(_window.min());

        { // text
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
                .pos(llsg::Vec2 { 5, 5});
            });
        }
        
        { // cursor
            static auto style = llsg::Style::defaultStyle();
            
            // opengl stuff
            auto &rend = llsg::opengl::getRenderer();
            auto bbox  = rend.bbox(llsg::Text().str(std::string(_text.begin(),_text.begin()+_cursor)), style);
            
            // rend.bbox
            
            auto sel = document
            .selectAll(llsg::isPath, llsg::iter(1,1))
            .data( std::vector<double> { bbox.max().x() + 1 });

            sel
            .exit()
            .remove([](llsg::Element* e) { e->remove(); });
            
            sel
            .enter()
            .append([](llsg::Element* parent, double x) { return &parent->asGroup().path(); });
            
            sel
            .call([&](llsg::Element* e, double x) {
                auto &p = e->asPath();
                p.clear()
                .moveTo({ 5 + x, 5 - 2.0})
                .lineTo({ 5 + x, 20})
                .style().color().reset(_parity ? llsg::Color(0.0f,0.0f,0.0f) : llsg::Color(1.0f));
            });

        }
        
        _canvas.markDirty(false);
        
    }
    
    void TextEdit::blink(int parity) {
        _parity = parity;
        _canvas.markDirty();
    }

    void TextEdit::onKeyPress(const App &app) {
        auto code = app.current_event_info.key_code;
        auto &modifiers = app.current_event_info.modifiers;
        if ( event::isprint(code) ) {
            _text.insert(_text.begin() + _cursor, ascii(code, modifiers));
            ++_cursor;
        }
        else if (code == event::KEY_BACKSPACE) {
            if (_cursor > 0) {
                _text.erase(_text.begin() + _cursor - 1);
                --_cursor;
            }
        }
        else if (code == event::KEY_LEFT) {
            if (_cursor > 0) {
                if (modifiers.control || modifiers.alt)
                    _cursor = 0;
                else
                    --_cursor;
            }
        }
        else if (code == event::KEY_RIGHT) {
            if (_cursor < _text.size()-1) {
                if (modifiers.control || modifiers.alt)
                    _cursor = _text.size();
                else
                    ++_cursor;
            }
        }
        _canvas.markDirty();
    }
    
}
