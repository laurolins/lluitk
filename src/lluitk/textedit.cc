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
//        std::cerr << "TextEdit::contains: p=" << p << " is in " << _window << (_window.contains(p) ? " yes" : " no") << std::endl;
//        std::cerr.flush();
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
        
        auto style = inheritedStyle();

        // default font etc
        _canvas.root.style().typeface().reset(style.typeface()());
        _canvas.root.style().fontSize().reset(style.fontSize()());
        _canvas.root.style().color().reset(style.fgcolor()());
        
        { // text
            auto sel = document
            .selectAll(llsg::isRect, llsg::iter(1,1))
            .data( std::vector<int> { 1 });
            
            sel
            .exit()
            .remove([](llsg::Element* e) { e->remove(); });
            
            sel
            .enter()
            .append([](llsg::Element* parent, const int& s) { return &parent->asGroup().rect(); });
            
            sel
            .call([&](llsg::Element* e, const int& s) {
                auto &r = e->asRectangle();
                r.pos({0,0}).size(_window.size()).style().color().reset(style.bgcolor()());
            });
        }

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
                t.str(s).pos(_offset);
            });
        }
        
        { // cursor
            // opengl stuff
            auto &rend = llsg::opengl::getRenderer();
            auto bbox  = rend.bbox(llsg::Text().str(std::string(_text.begin(),_text.begin()+_cursor)), _canvas.root.style());
            
            // rend.bbox
            
            auto sel = document
            .selectAll(llsg::isPath, llsg::iter(1,1))
            .data( _is_focused ? std::vector<double> { bbox.max().x() + 1 } : std::vector<double> {});

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
                .moveTo(Vec2{ _offset.x() + x, 0} )
                .lineTo(Vec2{ _offset.x() + x, _window.size().y()})
                .style().color().reset(_parity ? style.bgcolor()() : style.fgcolor()());
            });

        }
        
        _canvas.markDirty(false);
        
    }
    
    void TextEdit::blink(int parity) {
        _parity = parity;
        _canvas.markDirty();
    }

    const Vec2& TextEdit::offset() const {
        return _offset;
    }
    
    TextEdit&  TextEdit::offset(const Vec2& o) {
        _offset = o;
        return *this;
    }
    
    TextEdit& TextEdit::triggerFunction(TriggerFunction t) {
        _trigger = t;
        return *this;
    }

    void TextEdit::onMouseMove(const App &app) {
//        auto mouse_pos = app.current_event_info.mouse_position;
//        std::cerr << "TextEdit::onMouseMove: " << mouse_pos.x() << "," << mouse_pos.y() << std::endl;
    }

    void TextEdit::setKeyFocus(bool focused) {
        if (focused != _is_focused) {
            _is_focused = focused;
            _canvas.markDirty();
        }
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
            if (_cursor < _text.size()) {
                if (modifiers.control || modifiers.alt)
                    _cursor = _text.size();
                else
                    ++_cursor;
            }
        }
        else if (code == event::KEY_ENTER) {
            if (_trigger)
                _trigger(std::string(_text.begin(),_text.end()));
        }
        _canvas.markDirty();
    }
    
}
