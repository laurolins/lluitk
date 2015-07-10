#include <thread>
#include <chrono>

#include <GL/glew.h>

#include "lluitk/base.hh"
#include "lluitk/app.hh"
#include "lluitk/textedit.hh"
#include "lluitk/event.hh"
#include "lluitk/grid.hh"
#include "lluitk/os.hh"

#include "transition.hh"

int main() {

    auto &window = lluitk::os::graphics().window(200,200);
    
    auto app = lluitk::App();
    
    lluitk::os::event().callback([&app]( const ::lluitk::event::Event& e) {
        app.processEvent(e);
    });
    
    // create a container widget
    lluitk::TextEdit textedits[3];

    // grid widget
    lluitk::Grid     grid({1,3});
    
    textedits[0].style().fontSize().reset(lluitk::FontSize{24});
    // textedits[0].style().typeface().reset(lluitk::Typeface{"Monaco"});
    
    //
    grid.setCellWidget({0,0}, &textedits[0]);
    grid.setCellWidget({0,1}, &textedits[1]);
    grid.setCellWidget({0,2}, &textedits[2]);
    
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
