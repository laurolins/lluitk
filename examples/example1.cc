#include "lluitk/app.hh"
#include "lluitk/textedit.hh"

#include "os.hh"

int main() {

    auto &window = lluitk::os::graphics().window(640,480);
    auto app = lluitk::App();
    
    
    // create a container widget
    lluitk::TextEdit textedit;
    
    // set main window
    app.setMainWidget(&textedit);

    
    // bind window to current thread
    window.bind_to_thread();
    
    while (!window.done()) {
        
        // app.main_widget->render();
        
        lluitk::os::event().poll(); // poll events
    }

    return 0;
}
