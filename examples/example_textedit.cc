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

int main() {

    auto &window = lluitk::os::graphics().window(200,200);
    
    auto app = lluitk::App();
    
    const std::vector<llsg::Color> colors = {"#a6cee3","#1f78b4","#b2df8a","#33a02c","#fb9a99","#e31a1c","#fdbf6f","#ff7f00","#cab2d6","#6a3d9a","#ffff99","#b15928"};
    
    // create a container widget
    const int n = 3;
    lluitk::TextEdit textedits[n];
    for (auto i=0;i<n;++i) textedits[i].style().bgcolor().reset(colors[i]);

    // grid widget
    lluitk::grid2::Grid2 grid;
    
    grid.border_size(20);
    grid.margin_size(5);
    
    textedits[0].style().fontSize().reset(lluitk::FontSize{24});
    // textedits[0].style().typeface().reset(lluitk::Typeface{"Monaco"});
    
    //
    for (auto i=0;i<n;++i) { grid.insert(&textedits[i], i); }
    
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
    
    bool saved = false;
    char buffer[5000];
    lluitk::os::event().callback([&app,&buffer,&grid,&saved,&textedits]( const ::lluitk::event::Event& e) {
        if (e.getType() == lluitk::event::EVENT_KEY_PRESS && e.asKeyPress().key == lluitk::event::KEY_S) {
            auto n = grid.code(&buffer[0], 5000);
            std::cout << buffer << std::endl;
            std::cout << "Saved grid layout size if " << n << std::endl;
            saved = true;
        }
        else if (e.getType() == lluitk::event::EVENT_KEY_PRESS && e.asKeyPress().key == lluitk::event::KEY_L) {
            if (saved) {
                lluitk::grid2::Grid2 local_grid;
                auto error = lluitk::grid2::parse(buffer, local_grid);
                if (!error) {
                    // swap local grid
                    auto window = grid.window();
                    std::swap(grid, local_grid);
                    lluitk::grid2::NodeIterator it(grid.root());
                    lluitk::grid2::Node* node;
                    while ((node = it.next())) {
                        if (node->is_slot()) {
                            node->as_slot()->widget(&textedits[node->as_slot()->user_number()]);
                        }
                    }
                    grid.sizeHint(window);
                }
                else { std::cout << "error: " << error << std::endl; }
            }
        }
        else {
            app.processEvent(e);
        }
    });
    
    
    // app.
    
    // schedule transition

    
    bool update_cursor = true;
    int  parity        = 0;
    
    while (!window.done()) {
        
        /* Render here */
        glClearColor(0.0f,0.0f,0.0f,1.0f);
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
