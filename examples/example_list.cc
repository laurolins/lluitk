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
#include "lluitk/list.hh"

#include "llsg/llsg.hh"
#include "llsg/transition.hh"



struct Model {
    
    using key_type = std::string;

    using Size  = typename lluitk::list::Size;
    
    using Index = typename lluitk::list::Index;
    
    Model() {
        for (int i=0;i<3;++i) {
            _items.push_back(std::string("item ") + std::to_string(i+1));
        }
    }
    
    lluitk::list::Size size() const { return static_cast<Size>(_items.size()); }
    
    key_type           key(Index index) const { return _items.at(static_cast<size_t>(index)); }
    
    // map index into a geometrical element
    std::unique_ptr<llsg::Element> geometry(Index index, const lluitk::list::ListConfig& config, bool selected) const {

        auto k = key(index);
        
        std::unique_ptr<llsg::Element> result;
        
        result.reset(new llsg::Group());
        
        auto &g = result.get()->asGroup();
        
        g
        .rect()
        .size({config.window().width(),config.item_weight()})
        .style()
        .color()
        .reset(llsg::Color{selected ? 0.5f : 1.0f});

        g
        .text()
        .str(k)
        .pos({5, 5})
        .style().color().reset({0.0f});
        
        return std::move(result);
    }
    
    
    std::vector<std::string> _items;
};


int main() {

    Model model;

    using Key        = typename Model::key_type;
    using Lst        = lluitk::list::List<Model>;
    using Index      = lluitk::list::Index;
    using ListConfig = lluitk::list::ListConfig;
    using GeomMap    = Lst::geometry_map_type;
    
    GeomMap geomMap = [&model](Index index, const ListConfig& config, bool selected) {
        return model.geometry(index, config, selected);
    };
    
    Lst lst;
    lst.root().style().fontSize().reset(20);
    lst.item_weight(30);
    
    lst.geometryMap(geomMap).model(&model);

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
    grid.grid_style().clear(true).clear_color({1.0f});
    
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
        
        /* Render here */
        glClearColor(1.0f,1.0f,1.0f,1.0f);
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




    
    
    
    
    
    
    
    
    
    
    
    
    
    
