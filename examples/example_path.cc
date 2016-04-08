#include <thread>
#include <chrono>

#include <GL/glew.h>

#include "lluitk/base.hh"
#include "lluitk/app.hh"
#include "lluitk/textedit.hh"
#include "lluitk/event.hh"
#include "lluitk/grid2.hh"
#include "lluitk/os.hh"

#include "llsg/transition.hh"
#include "llsg/llsg_opengl.hh"

using lluitk::Point;
using lluitk::Window;
using lluitk::Vec2;

struct Panel: public lluitk::SimpleWidget {
public:
    
    llsg::Group _group;
    
    bool _dirty { true };
    
    lluitk::Window _window;
    
    float _line_width { 100.0f };
    float _period     { 15.0f };
    
    bool _dragging { false };
    
    enum Mode { UPDATE_WIDTH, UPDATE_PERIOD };
    
    Mode _mode;
    
public:
    
    Panel() = default;
    
    bool contains(const lluitk::Point& p) const { return _window.contains(p); }
    
    void sizeHint(const lluitk::Window &w) { _window=w; _dirty=true; };
    
    void render() {
        
        if (_window.area() == 0.0) return;
        
        if (_dirty) {
            
            // prepare scene
            _group.removeAll();

            auto n = 2;

            Vec2 margin(30,30);
            Window plot_area(_window.xy() + margin, _window.XY() - margin);
            
            auto f = [n, plot_area](int i) {
                auto dx = (double) 1 / (double) n;
                auto x  = i * dx;
                auto y  = (i % 2) ? (0.5*(1.0-x)) : (1.0-0.5*(1.0-x));
                auto p = Vec2(x,y);
                return plot_area.xy() + plot_area.size() * p;
            };

            
            auto &path = _group.path();

            for (auto i=0;i<=n;++i) {
                auto p = f(i);
                if (i > 0) path.lineTo(p);
                else path.moveTo(p);
            }
            
            path.style().color().reset({0.0f,0.0f,1.0f});
            path.style().lineWidth().reset(_line_width);
            
            // const std::vector<llsg::Color> colors = {"#a6cee3","#1f78b4","#b2df8a","#33a02c","#fb9a99","#e31a1c","#fdbf6f","#ff7f00","#cab2d6","#6a3d9a","#ffff99","#b15928"};
            
            llsg::PathPattern pp;
            pp.insert({"#b2df8a"}, 10);
            pp.insert({"#000000"}, 1);
            pp.insert({"#e31a1c"}, 10);
            pp.insert({"#000000"}, 1);
            
            pp.period(_period);
            
            path.path_pattern().reset(new llsg::PathPattern(std::move(pp)));
         
            _dirty = false;
            
        }
        
        // render
        auto &renderer = llsg::opengl::getRenderer();
        renderer.render(_group, llsg::Transform(), _window);

    }
    
    void onMousePress(const lluitk::App& app) {
        _dragging=true;
        
        _mode = app.current_event_info.left_button() ? UPDATE_WIDTH : UPDATE_PERIOD;
        
        app.lock(this);
        app.finishEventProcessing();
    }
    
    void onMouseMove(const lluitk::App& app) {
        if (_dragging) {
            auto p1 = app.current_event_info.mouse_position;
            auto p0 = app.last_event_info.mouse_position;
            
            auto dx = (p1.x() - p0.x()) * (app.current_event_info.modifiers.shift ? 1.0f : 0.2f);
            
            if (_mode == UPDATE_WIDTH) {
                _line_width += dx;
                if (_line_width < 0.1f) _line_width = 0.1;
            }
            else {
                _period += dx;
                if (_period < 0.1f) _period = 0.1;
            }
            
            _dirty = true;
        }
        app.finishEventProcessing();
    }

    void onMouseRelease(const lluitk::App& app) {
        if (_dragging) {
            _dragging=false;
            app.lock();
            app.finishEventProcessing();
        }
    }

    
};




int main() {
    
    auto &window = lluitk::os::graphics().window(800,800);
    
    auto app = lluitk::App();
    
    Panel panel;
    
    // set main window
    app.setMainWidget(&panel);
    
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
    
    lluitk::os::event().callback([&app,&panel]( const ::lluitk::event::Event& e) {
//        if (e.getType() == lluitk::event::EVENT_KEY_PRESS && e.asKeyPress().key == lluitk::event::KEY_S) {
//            auto n = grid.code(&buffer[0], 5000);
//            std::cout << buffer << std::endl;
//            std::cout << "Saved grid layout size if " << n << std::endl;
//            saved = true;
//        }
//        else if (e.getType() == lluitk::event::EVENT_KEY_PRESS && e.asKeyPress().key == lluitk::event::KEY_L) {
//            if (saved) {
//                lluitk::grid2::Grid2 local_grid;
//                auto error = lluitk::grid2::parse(buffer, local_grid);
//                if (!error) {
//                    // swap local grid
//                    auto window = grid.window();
//                    std::swap(grid, local_grid);
//                    lluitk::grid2::NodeIterator it(grid.root());
//                    lluitk::grid2::Node* node;
//                    while ((node = it.next())) {
//                        if (node->is_slot()) {
//                            node->as_slot()->widget(&textedits[node->as_slot()->user_number()]);
//                        }
//                    }
//                    grid.sizeHint(window);
//                }
//                else { std::cout << "error: " << error << std::endl; }
//            }
//        }
//        else {
            app.processEvent(e);
//        }
    });
    
    
    // app.
    
    // schedule transition
    
    
    bool update_cursor = true;
    int  parity        = 0;
    
    while (!window.done()) {
        
        /* Render here */
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glViewport(0,0,window.framebuffer_width,window.framebuffer_height);
        
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
