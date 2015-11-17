#pragma once

#include <cassert>

#include "base.hh"
#include "event.hh"
#include "app.hh"
#include "simple_widget.hh"

#include "llsg/llsg.hh"
#include "llsg/llsg_opengl.hh"
#include "llsg/transition.hh"

namespace lluitk {
    
    namespace list {
        
        using Size  = std::int64_t;
        using Index = std::int64_t;
        
        //----------------
        // SpeedupWheel
        //----------------
        
        struct SpeedupWheel {
            SpeedupWheel();
            double speedup(llsg::Vec2 move);
            double _factor { 1.0 };
            int           _repeats { 0 };
            llsg::Vec2    _move;
            std::uint64_t _ts { 0ULL };
            std::uint64_t _starttime { 0ULL };
        };
        
        //----------------
        // ListConfig
        //----------------
        
        struct ListConfig {
            ListConfig() = default;
            
            bool                  vertical()    const { return _vertical; }
            float                 item_weight() const { return _item_weight; }
            
            ListConfig&           item_weight(float w) { _item_weight = w; return *this; }

            const llsg::Vec2&     position() const { return _position; }
            const lluitk::Window& window() const { return _window; }

            llsg::Vec2&           position() { return _position; }
            lluitk::Window&       window() { return _window; }

            ListConfig&           position(const llsg::Vec2& pos) { _position = pos; return *this; }
            ListConfig&           window(const lluitk::Window& window) { _window = window; return *this; }
            
            bool            _vertical { true };
            float           _item_weight { 20.0f };

            llsg::Vec2      _position; // tranlate the items rectangle
            lluitk::Window  _window;   // current visible area
        };
        
        /*!
         *
         * An Item should be just a key
         *
         *
         * item -> geometry
         *
         *
         * how can a list change:
         *
         *    permute or change the ordering of items
         *    addition/removal of items (together with permutation)
         *
         *    a simple model would be to query which items are in a certain
         *    ordering range... from there we could map the keys and do the
         *    necessary transition!
         *
         *
         *
         *
         *
         * animation could be staged into:
         *
         *     disappear
         *     permute
         *     appear
         *
         */
        
        //
        // A model has to have the following
        //
        // item type should be a lightweight copy friendly "key"
        //
        // typename Model::item_type;
        //
        // Model::item_type key(Index index) const;
        //             Size size()           const;
        //
        //
        
        //----------------------------------------------------------------------------
        // List
        //----------------------------------------------------------------------------
        
        template <typename Model>
        struct List: public lluitk::SimpleWidget {
        public:
            using key_type          = typename Model::key_type; // more

            using geometry_map_type = std::function<std::unique_ptr<llsg::Element>(Index, const ListConfig&, bool)>;
        
            using trigger_function_type = std::function<void(const List& list)>;

        public:

            List() = default;
            List& model(Model *model);
            List& geometryMap(geometry_map_type f);
            
            Index selectect_index() const { return _selectedIndex; }

            const Model* model() const { return _model; }
            
            Model* model() { return _model; }
        
        public:
            void onMouseWheel(const lluitk::App &app);
            void onMousePress(const lluitk::App &app);
            void onMouseMove(const lluitk::App &app);
            void onMouseRelease(const lluitk::App &app);

        public:
            
            void render();
            void prepare();
            
            //
            // if double click the same item or
            // press enter with a selected item
            //
            void register_trigger_function(trigger_function_type f);
            void remove_trigger_function();

            bool contains(const lluitk::Point& p) const;
            void sizeHint(const lluitk::Window &window);
            
            llsg::Group& root() { return _root; }
            ListConfig&  config() { return _config; }
            
            void item_weight(float w) { _config.item_weight(w); _dirty = true; }

            llsg::Color scrollbar_bar_color() const { return _scrollbar_bar_color; }
            llsg::Color scrollbar_cursor_color() const { return _scrollbar_cursor_color; }

            List& scrollbar_bar_color(const llsg::Color& c) { _scrollbar_bar_color = c; return *this; }
            List& scrollbar_cursor_color(const llsg::Color& c) { _scrollbar_cursor_color = c; return *this; }
            
        private:
            
            void _trigger();
            
        private:
            trigger_function_type _trigger_function;
            SpeedupWheel      _speedup_wheel;

            Model*            _model { nullptr };
            geometry_map_type _geometry_map; // not defined at first

            ListConfig        _config;
            bool              _dirty { true };
            
            llsg::Group       _root;
            llsg::Group       _scroller_root;
            
            Index             _selectedIndex { -1 };
            Microseconds      _last_press_timestamp { 0 };
            bool              _dragging { false };
            
            
            llsg::Color       _scrollbar_bar_color    { 0.8f };
            llsg::Color       _scrollbar_cursor_color { 1.0f };
        };
        
        //---------------------------------------------------------------------------
        // List Implementation
        //---------------------------------------------------------------------------

        template <typename M>
        void List<M>::remove_trigger_function() {
            trigger_function_type f;
            _trigger_function = f;
        }

        template <typename M>
        void List<M>::_trigger() {
            if (_trigger_function)
                _trigger_function(*this);
        }
        
        template <typename M>
        void List<M>::register_trigger_function(trigger_function_type f) {
            _trigger_function = f;
        }
        
        template <typename M>
        bool List<M>::contains(const lluitk::Point& p) const {
            return _config.window().contains(p);
        }
        
        template <typename M>
        void List<M>::sizeHint(const lluitk::Window &window) {
            _dirty  = true;
            
            assert(_config.vertical());
            
            // std::cerr << "new list size: " << window << std::endl;
            
            _config.window(window);

            auto offset = -_config.position().y();
            auto length = _model->size() * _config.item_weight();
            auto max_offset = std::max(0.0,static_cast<double>((length - _config.window().height())));
            _config.position().y(-std::min(std::max(0.0, offset),max_offset));

        }
        
        template <typename M>
        void List<M>::onMousePress(const lluitk::App &app) {
            auto &window = _config.window();
            auto window_pos = app.current_event_info.mouse_position - window.min();

            assert(_config.vertical());
            
            auto t1    = app.current_event_info.timestamp();
            auto delta = t1 - _last_press_timestamp;
            // std::cerr << delta << std::endl;
            
            llsg::GeometricTests g;
            auto e = g.firstHit(llsg::Vec2{ (double) window_pos.x(), (double) window_pos.y() }, _scroller_root);
            if (e && any::can_cast<std::string>(e->data())) {
                auto command = any::any_cast<std::string>(e->data());
                auto y = (window.height() - window_pos.y())/window.height();
                auto i_mid = static_cast<Index>(std::round(y * _model->size()));
                auto rows_per_window = (window.height() / _config.item_weight());

                
                auto candidate_offset = std::floor((i_mid - rows_per_window/2.0f) * _config.item_weight());
                auto max_offset = static_cast<double>((_model->size() * _config.item_weight() - _config.window().height()));
                _config.position().y(-std::min(std::max(0.0,candidate_offset),max_offset));
                
                _dirty    = true;
                _dragging = true;
                app.lock(this);
            }
            else {
                auto y = (window.height() - window_pos.y()) - _config.position().y();
                Index i = static_cast<Index>(y / _config.item_weight());
                if (i != _selectedIndex) {
                    _selectedIndex = (i < _model->size()) ? i : -1;
                    _dirty = true;
                }
                else if (delta < 200000)  { // 0.2 seconds
                    _trigger();
                }
            }
            _last_press_timestamp = app.current_event_info.timestamp();
        }
        
        template <typename M>
        void List<M>::onMouseMove(const lluitk::App &app) {
            if (_dragging) {
                auto window_pos = app.current_event_info.mouse_position - _config.window().min();
                auto y = (_config.window().height() - window_pos.y())/_config.window().height();
                auto i_mid = static_cast<Index>(std::round(y * _model->size()));
                auto rows_per_window = (_config.window().height() / _config.item_weight());

                auto candidate_offset = std::floor((i_mid - rows_per_window/2.0f) * _config.item_weight());
                auto max_offset = static_cast<double>((_model->size() * _config.item_weight() - _config.window().height()));
                _config.position().y(-std::min(std::max(0.0,candidate_offset),max_offset));
                
                _dirty = true;
            }
        }

        template <typename M>
        void List<M>::onMouseRelease(const lluitk::App &app) {
            if (_dragging) {
                _dragging = false;
                app.lock();
            }
        }
    
        template <typename M>
        auto List<M>::model(M *model) -> List& {
            _model = model;
            _dirty = true;
            return *this;
        }
        
        template <typename M>
        auto List<M>::geometryMap(geometry_map_type f) -> List& {
            _geometry_map = f;
            _dirty = true;
            return *this;
        }
        
        template <typename M>
        void List<M>::render() {
            auto& window = _config.window();

            if (window.width() == 0 || window.height() == 0)
                return;

            if (_dirty)
                prepare();
            
            // get llsg renderer and
            // _scene.img().key(resloc::getResourcePath("logo/nanocubes-blue-name-logo.png")).coords(llsg::Quad{0.0f,0.0f,600.0f,180.0f});
            llsg::opengl::getRenderer().render(_root, llsg::Transform{}.translate(window.min()), window, false);
            llsg::opengl::getRenderer().render(_scroller_root, llsg::Transform{}.translate(window.min()), window, false);
        }
        
        template <typename M>
        void List<M>::onMouseWheel(const lluitk::App &app) {
            auto delta = app.current_event_info.mouse_wheel_delta;
            if (_config.vertical()) {
                
                auto factor = _speedup_wheel.speedup(delta);
                
                auto dy = std::round(factor * delta.y());
                
                //
                // each wheel event that was in the same direction
                // of the previous wheel event and within a threshold
                // from the previous event might get a multiplicity
                // factor: every
                //
                
                
                _config.position().yinc(dy);
                // max position on y is
                float miny = -(_model->size() * _config.item_weight() - _config.window().height());
                if (_config.position().y() < miny) {
                    _config.position().y(miny);
                }
                if (_config.position().y() > 0) {
                    _config.position().y(0);
                }
                _dirty = true;
            }
            
        }
        
        template <typename M>
        void List<M>::prepare() {

            auto &window   = _config.window();
            auto &position = _config.position();
            auto  vertical = _config.vertical();
              
            //
            _root.removeAll();
        
            if (!_model || _model->size() == 0)
                return;
        
            // regenerate all the geometry from scratch (improve this later)
        
            //
            // given the current position,
            // figure out item range that is visible
            //
            Index i0 = static_cast<Index>( (vertical ? -position.y() : position.x()) / _config.item_weight());
            Index i1 = static_cast<Index>((vertical ?
                                           window.height() - position.y() :
                                           window.width()  + position.x()) / _config.item_weight());
            if (i1 >= _model->size()) {
                i1 = static_cast<Index>(_model->size()) - 1;
            }
            if (i0 > i1) {
                i0 = i1;
            }
        

            { // prepare geometry of items
                // std::cout << "i0: " << i0 << "  i1: " << i1 << " pos: " << _position << std::endl;
                for (auto i=i0;i<=i1;++i) {
                    // std::cout << i << std::endl;
                    auto elem_p = _geometry_map(i, _config, i == _selectedIndex);
                    auto &g = _root.g();
                    g.data(_model->key(i)); // associate key

                    g.transform(llsg::Transform()
                                .translate(vertical ?
                                           llsg::Vec2(0, window.height() - (i+1) * _config.item_weight()) :
                                           llsg::Vec2(i * _config.item_weight(), 0)));
                    g.append(std::move(elem_p));
                }
                _root.identity().translate({-position.x(),-position.y()});
            }
        
            { // prepare scroller (draw two rectangles if needed)
                _scroller_root.removeAll();
                
                //
                // length == n * w
                // visible_length = window.height
                // cursor length
                
                auto length         = _model->size() * _config.item_weight();
                auto visible_length = (vertical ? window.height() : window.width());
                // (vertical ? window.height() : window.width());
                if (visible_length < length) {

                    auto cursor_length    = std::floor(visible_length/length * visible_length);
                    auto cursor_pos       = std::floor(-position.y()/length * visible_length);// (i0 * _config.item_weight())/length * active_length;
                    auto cursor_width     = 6.0f;
                
                    auto bar_width        = 10.0f;
                    auto bar_pos          = _config.vertical() ? llsg::Vec2{window.width()-bar_width,0} : llsg::Vec2{0,0};
                    auto bar_size         = _config.vertical() ? llsg::Vec2{bar_width,window.height()} : llsg::Vec2{window.width(),bar_width};
                
                    auto cursor_margin    = (bar_width-cursor_width)/2.0f;
                
                    llsg::Vec2 p0; //    { _window.min() };
                    auto size = (vertical ? 
                                 llsg::Vec2 {cursor_width,cursor_length} : 
                                 llsg::Vec2 {cursor_length,cursor_width});
                    if (_config.vertical()) {
                        p0.xinc(window.width() - bar_width + cursor_margin);
                        p0.yinc(window.height() - cursor_pos - cursor_length);
                    }
                    else {
                        p0.xinc(cursor_pos);
                        p0.yinc(cursor_margin);
                    }
                    _scroller_root
                        .rect()
                        .pos(bar_pos)
                        .size(bar_size)
                        .data(std::string("bar"))
                        .style()
                        .color()
                        .reset(llsg::Color{0.5f,0.5f,0.5f,1.0f});
                
                    _scroller_root
                        .rect()
                        .pos(p0)
                        .size(size)
                        .data(std::string("cursor"))
                        .style().color().reset(llsg::Color{1.0f,1.0f,1.0f,1.0f});
                }
            }
        
            _dirty = false;

        } // _prepare
        
    } // list
    
} // lluitk
