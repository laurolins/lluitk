#include <thread>
#include <chrono>

#include <functional>
#include <fstream>


#include <GL/glew.h>

#include "lluitk/base.hh"
#include "lluitk/app.hh"
#include "lluitk/textedit.hh"
#include "lluitk/event.hh"
#include "lluitk/grid.hh"
#include "lluitk/os.hh"

#include "llsg/llsg.hh"
#include "llsg/transition.hh"



static std::uint64_t RENDER_LOOP_ITERATON = 0;

namespace list {

    using Index = int;
    
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
    
   
    /*!
     * Notion of
     *
     */
    template <typename Item, typename Model>
    struct List: public lluitk::SimpleWidget {
    public:
        using geometry_map_type = std::function<std::unique_ptr<llsg::Element>(const Item&,
                                                                               Index,
                                                                               const ListConfig&,
                                                                               bool)>;
    public:
        List() = default;
        List& model(Model *model) {
            _model = model;
            _dirty = true;
            return *this;
        }
        List& geometryMap(geometry_map_type f) {
            _geometry_map = f;
            _dirty = true;
            return *this;
        }
        
    public:
        void onMouseWheel(const lluitk::App &app);
        void onMousePress(const lluitk::App &app);
    public:
        void render();
        void prepare();
        bool contains(const lluitk::Point& p) const;
        void sizeHint(const lluitk::Window &window);
    public:
        Model*            _model { nullptr };
        geometry_map_type _geometry_map; // not defined at first
        llsg::Vec2        _position; // count from the top left
        lluitk::Window    _window;   // current visible area
        ListConfig        _config;
        bool              _dirty { true };
        llsg::Group       _root;
        Index             _selectedIndex { -1 };
    };

    //
    // List Implementation
    //

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

    
    
    std::ofstream& wheelos() {
        static std::unique_ptr<std::ofstream> ptr;
        if (!ptr) {
            ptr.reset(new std::ofstream("/Users/llins/Desktop/wheel.psv"));
            auto &os = *ptr.get();
            os << "time|delta|iter" << std::endl;
        }
        return *ptr.get();
    }
    
    
    template <typename I, typename M>
    void List<I,M>::render() {
        if (_dirty)
            prepare();
        
        // get llsg renderer and
        // _scene.img().key(resloc::getResourcePath("logo/nanocubes-blue-name-logo.png")).coords(llsg::Quad{0.0f,0.0f,600.0f,180.0f});
        llsg::opengl::getRenderer().render(_root, llsg::Transform{}.translate(_window.min()), _window, false);
    }
    
    
    struct SpeedupWheel {
        SpeedupWheel() {
            _starttime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        }
        double speedup(llsg::Vec2 move) {
            auto current_ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();;

            wheelos() << (current_ts - _starttime) << "|" << move.y() << "|" << RENDER_LOOP_ITERATON << std::endl;

            if (move != _move) {
                _factor  = 1.0;
                _repeats = 1;
                _move    = move;
                _ts      = current_ts;
            }
            else {
                auto dt = current_ts - _ts;
                if (dt < 100000000ULL) { // 25ms
                    if ((_repeats % 10) == 0) {
                        if (_factor < 128) {
                            _factor *= 2;
                            std::cout << "speed up " << _factor << " reps: " << _repeats << std::endl;
                        }
                    }
                    _ts = current_ts;
                    ++_repeats;
                }
                else {
                    std::cout << "cut move" << std::endl;
                    _factor  = 1.0;
                    _repeats = 1;
                    _move    = move;
                    _ts      = current_ts;
                }
            }
            return _factor;
        }
        double _factor { 1.0 };
        int           _repeats { 0 };
        llsg::Vec2    _move;
        std::uint64_t _ts { 0ULL };
        std::uint64_t _starttime { 0ULL };
    };

    template <typename I, typename M>
    void List<I,M>::onMouseWheel(const lluitk::App &app) {
        auto delta = app.current_event_info.mouse_wheel_delta;
        if (_config.vertical) {
            
            static SpeedupWheel speedup;
            
            auto factor = speedup.speedup(delta);

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
            auto elem_p = _geometry_map(item, i, _config, i == _selectedIndex);
            auto &g     = _root.g();
            g.transform(llsg::Transform().translate(_config.vertical ?
                                                    llsg::Vec2(0, _window.height() - (i+1) * _config.width) :
                                                    llsg::Vec2(i * _config.width, 0)));
            g.append(std::move(elem_p));
        }
        _root.identity().translate({-_position.x(),-_position.y()});
        
        _dirty = false;
    }

}

int main() {

    std::vector<std::string> items;
    
    for (int i=0;i<1000;++i) {
        items.push_back(std::string("item ") + std::to_string(i+1));
    }
    
    using Item    = std::string;
    using Model   = std::vector<Item>;
    using Lst     = list::List<Item, Model>;
    using GeomMap = Lst::geometry_map_type;
    using Index   = list::Index;
    
    GeomMap geomMap = [](const Item& item, Index index, const list::ListConfig& config, bool selected) {
        std::unique_ptr<llsg::Element> result;
        result.reset(new llsg::Group());
        auto &g = result.get()->asGroup();
        g
        .rect()
        .size({1000,config.width})
        .style().color().reset(llsg::Color{selected ? 0.5f : 0.0f});
        g.text().str(item).pos({5, 5});
        return std::move(result);
    };
    
    Lst lst;
    lst.geometryMap(geomMap).model(&items);

    auto &window = lluitk::os::graphics().window(200,200);
    
    auto app = lluitk::App();
    
    lluitk::os::event().callback([&app]( const ::lluitk::event::Event& e) {
        app.processEvent(e);
    });
    
    // grid widget
    lluitk::Grid     grid({1,1});
    
    // textedits[0].style().typeface().reset(lluitk::Typeface{"Monaco"});
    
    //
    grid.setCellWidget({0,0}, &lst);
    
    grid.setExternalHandleFixedSize(30).setInternalHandleFixedSize(30);
    
    
    grid.movableSplitters(false);
    
    // create a container widget
    // grid.sizeHint(lluitk::Window{lluitk::Point{0,0},lluitk::Point{200,200}});
    
    // set main window
    app.setMainWidget(&grid);
    
    // bind window to current thread
    window.bind_to_thread();
    
    // signal app to resize widgets
    app.processEvent(lluitk::event::WindowResize(
                                                 lluitk::Size {
                                                     (double)window.framebuffer_width,
                                                     (double)window.framebuffer_height
                                                 } ) );
    
    // set coef for retina display
    // llsg::opengl::getRenderer()._resolution_factor = window.window_to_framebuffer_factor;
    
    
    
    
    // schedule transition
    
    
    bool update_cursor = true;
    int  parity        = 0;
    
    while (!window.done()) {
        
        ++RENDER_LOOP_ITERATON;
        
        /* Render here */
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glViewport(0,0,window.framebuffer_width,window.framebuffer_height);
        
        // glShadeModel(GL_SMOOTH);
        // glEnable(GL_MULTISAMPLE);
        // glEnable( GL_POLYGON_SMOOTH );
        // glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        // glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window.framebuffer_width,
                0, window.framebuffer_height,
                0, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        app.main_widget->render();
        
        window.swap_buffers();
        
        lluitk::os::event().poll(); // poll events
        
        transition::getTransitionEngine().update();
        
        if (update_cursor) {
            transition::getTransitionEngine()
            .addTransitionToStandByList(transition::Transition([&](double t) {
                if (t == 1.0) {
                    parity = 1 - parity;
                    if (app._key_focus_widget) {
                        app._key_focus_widget->blink(parity);
                    }
                    update_cursor = true;
                }
            }).duration(transition::Duration{500})).startStandByTransitions();
            update_cursor = false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    }
    
    return 0;
}
    
    
    
    
    
    
    
    
    //    //
    //    // we need to provide a function that given an item and some parameters
    //    // generates a visualization for it
    //    //
    //
    //    lluitk::List<Item, Model> list;
    //
    //    //
    //    // when a new model comes, don't assume anything
    //    // recompute the cached geometry of any visible item
    //    // the ones that have the same key just fade out - fade in
    //    //
    //    list.model(items);
    //
    //    //
    //    // when a new item to geometry comes recache everything
    //    //
    //    list.itemToGeometry(function);
    //
    //    //
    //    // there will be a group and we will need to move stuff
    //    // we can add the signals later
    //    //
    //
    //
    //
    //
    //    // model
    //
    //    //
    //    list.update(items);
    
    
    
    
//
// a vector could be a model!!
// signal that model changed
//
//
//struct Model {
//    size()
//    Item operator[]
//};
//
//    using geometry_map_type = std::function<  std::unique_ptr<llsg::Element>(const Item& item, Index index, const ListConfig& config, bool selected)>;




    
    
    
    
    
    
    
    
    
    
    
    
    
    
