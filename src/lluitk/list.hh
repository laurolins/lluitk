#pragma once

#include "lluitk/base.hh"
#include "lluitk/event.hh"
#include "lluitk/app.hh"
#include "lluitk/simple_widget.hh"

#include "llsg/llsg.hh"
#include "llsg/llsg_opengl.hh"
#include "llsg/transition.hh"

namespace lluitk {
    
    namespace list {
        
        using Index = int;
        
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
            bool   vertical { true };
            float  width    { 20.0f };
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
        
        
        //----------------------------------------------------------------------------
        // List
        //----------------------------------------------------------------------------
        
        template <typename Item, typename Model>
        struct List: public lluitk::SimpleWidget {
        public:
            using geometry_map_type = std::function<std::unique_ptr<llsg::Element>(const Item&, Index, const List&, bool)>;
        public:
            List() = default;
            List& model(Model *model);
            List& geometryMap(geometry_map_type f);
        public:
            void onMouseWheel(const lluitk::App &app);
            void onMousePress(const lluitk::App &app);
        public:
            void render();
            void prepare();
            bool contains(const lluitk::Point& p) const;
            void sizeHint(const lluitk::Window &window);
        public:
            
            SpeedupWheel      _speedup_wheel;
            
            Model*            _model { nullptr };
            geometry_map_type _geometry_map; // not defined at first
            llsg::Vec2        _position; // count from the top left
            lluitk::Window    _window;   // current visible area
            ListConfig        _config;
            bool              _dirty { true };
            llsg::Group       _root;
            Index             _selectedIndex { -1 };
        };
        
        //---------------------------------------------------------------------------
        // List Implementation
        //---------------------------------------------------------------------------
        
        template <typename I, typename M>
        bool List<I,M>::contains(const lluitk::Point& p) const {
            return _window.contains(p);
        }
        
        template <typename I, typename M>
        void List<I,M>::sizeHint(const lluitk::Window &window) {
            _dirty  = true;
            _window = window;
        }
        
        template <typename I, typename M>
        void List<I,M>::onMousePress(const lluitk::App &app) {
            auto window_pos = app.current_event_info.mouse_position - _window.min();
            if (_config.vertical) {
                auto y = (_window.height() - window_pos.y()) - _position.y();
                Index i = static_cast<Index>(y / _config.width);
                _selectedIndex = i;
                _dirty = true;
            }
        }
        
        template <typename I, typename M>
        auto List<I,M>::model(M *model) -> List& {
            _model = model;
            _dirty = true;
            return *this;
        }
        
        template <typename I, typename M>
        auto List<I,M>::geometryMap(geometry_map_type f) -> List& {
            _geometry_map = f;
            _dirty = true;
            return *this;
        }
        
        template <typename I, typename M>
        void List<I,M>::render() {
            if (_dirty)
                prepare();
            
            // get llsg renderer and
            // _scene.img().key(resloc::getResourcePath("logo/nanocubes-blue-name-logo.png")).coords(llsg::Quad{0.0f,0.0f,600.0f,180.0f});
            llsg::opengl::getRenderer().render(_root, llsg::Transform{}.translate(_window.min()), _window, false);
        }
        
        template <typename I, typename M>
        void List<I,M>::onMouseWheel(const lluitk::App &app) {
            auto delta = app.current_event_info.mouse_wheel_delta;
            if (_config.vertical) {
                
                auto factor = _speedup_wheel.speedup(delta);
                
                auto dy = std::round(factor * delta.y());
                
                //
                // each wheel event that was in the same direction
                // of the previous wheel event and within a threshold
                // from the previous event might get a multiplicity
                // factor: every
                //
                
                
                _position.yinc(dy);
                // max position on y is
                float miny = -(_model->size() * _config.width - _window.height());
                if (_position.y() < miny) {
                    _position.y(miny);
                }
                if (_position.y() > 0) {
                    _position.y(0);
                }
                _dirty = true;
            }
            
        }
        
        template <typename I, typename M>
        void List<I,M>::prepare() {
            //
            _root.removeAll();
            
            if (!_model || _model->size() == 0)
                return;
            
            
            
            // regenerate all the geometry from scratch (improve this later)
            
            //
            // figure out item range
            //
            
            Index i0 = static_cast<Index>(
                                          (_config.vertical ?
                                           -_position.y() :
                                           _position.x()) / _config.width);
            
            Index i1 = static_cast<Index>((_config.vertical ?
                                           _window.height() - _position.y() :
                                           _window.width()  + _position.x()) / _config.width);
            
            //
            if (i1 > _model->size()) {
                i1 = static_cast<Index>(_model->size()) - 1;
            }
            
            // std::cout << "i0: " << i0 << "  i1: " << i1 << " pos: " << _position << std::endl;
            
            //
            // all right
            //
            std::vector<I> items;
            items.reserve(i1-i0+1);
            for (auto i=i0;i<=i1;++i) {
                // std::cout << i << std::endl;
                auto item   = _model->operator[](i);
                auto elem_p = _geometry_map(item, i, *this, i == _selectedIndex);
                auto &g     = _root.g();
                g.transform(llsg::Transform().translate(_config.vertical ?
                                                        llsg::Vec2(0, _window.height() - (i+1) * _config.width) :
                                                        llsg::Vec2(i * _config.width, 0)));
                g.append(std::move(elem_p));
            }
            _root.identity().translate({-_position.x(),-_position.y()});
            
            _dirty = false;
        }
        
    } // list
    
} // lluitk
