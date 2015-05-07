#include <thread>
#include <chrono>

#include <GL/glew.h>

#include "lluitk/app.hh"
#include "lluitk/textedit.hh"
#include "lluitk/event.hh"

#include "os.hh"

int main() {

    auto &window = lluitk::os::graphics().window(640,480);
    
    auto app = lluitk::App();
    
    lluitk::os::event().callback([&app](lluitk::event::Event* e) {
        app.processEvent(e);
    });
    
    // create a container widget
    lluitk::TextEdit textedit;
    textedit.sizeHint(lluitk::Window{lluitk::Point{0,0},lluitk::Point{640,480}});
    // set main window
    app.setMainWidget(&textedit);
    
    // bind window to current thread
    window.bind_to_thread();
    
    while (!window.done()) {
        
        /* Render here */
        glClearColor(1.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glViewport(0,0,window.width,window.height);
        
        glShadeModel(GL_SMOOTH);
        glEnable(GL_MULTISAMPLE);
        glEnable( GL_POLYGON_SMOOTH );
        glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window.width,
                0, window.height,
                0, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        app.main_widget->render();
        
        window.swap_buffers();
        
        lluitk::os::event().poll(); // poll events
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    }

    return 0;
}
