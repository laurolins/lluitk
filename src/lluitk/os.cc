#include "os.hh"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace lluitk {
    
    namespace os {
        
        
        inline GLFWwindow* glfwwin(WindowHandle h) {
            return static_cast<GLFWwindow*>(h);
        }
        
        inline WindowHandle handle(GLFWwindow* w) {
            return static_cast<WindowHandle>(w);
        }
        
        //------------------------------------------------------------------------------
        // OpenGL Context and Window (which might be visible or not)
        //------------------------------------------------------------------------------
        
        Window::Window(int width, int height, bool visible, Window *parent):
        width(width),
        height(height),
        visible(visible),
        parent(parent)
        {
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_VISIBLE, visible ? GL_TRUE : GL_FALSE);
            
            auto window = glfwCreateWindow(width, height, "", NULL, (parent ? glfwwin(parent->handle) : NULL));
            if (!window) {
                throw std::runtime_error("could not create window");
            }
            
            handle = static_cast<WindowHandle>(window);
            
            // int framebuffer_width, framebuffer_height;
            glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height); // get the actual window size
            
            this->window_to_framebuffer_factor = framebuffer_width / width;
            
            if (framebuffer_height / height != this->window_to_framebuffer_factor)
                throw std::runtime_error("inconsistent window_to_framebuffer_factor");
            
            // this->framebuffer_width = this->window_to_framebuffer_factor * width;
        }
        
        Window::~Window() {
            auto window = static_cast<GLFWwindow*>(handle);
            glfwDestroyWindow(window);
        }
        
        void Window::bind_to_thread() {
            auto window = static_cast<GLFWwindow*>(handle);
            glfwMakeContextCurrent(window);
            glewInit();
        }
        
        bool Window::done() {
            auto window = static_cast<GLFWwindow*>(handle);
            return glfwWindowShouldClose(window);
        }
        
        void Window::setTitle(const std::string &title) {
            auto window = static_cast<GLFWwindow*>(handle);
            glfwSetWindowTitle(window, title.c_str());
        }
        
        void Window::swap_buffers() {
            auto window = static_cast<GLFWwindow*>(handle);
            glfwSwapBuffers(window);
        }
        
        //------------------------------------------------------------------------------
        // GraphicsLayer
        //------------------------------------------------------------------------------
        
        void error_callback(int error, const char* description)
        {
            std::cerr << "error: " << std::string(description) << std::endl;
        }
        
        GraphicsLayer::GraphicsLayer() {
            // initialize glfw
            glfwSetErrorCallback(error_callback);
            
            auto ok = glfwInit();
            if (!ok) {
                throw std::runtime_error("oops");
            }
            
            glfwWindowHint(GLFW_SAMPLES, 8);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            //    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            
            // glfwSwapInterval(0); // vsync?
        }
        
        GraphicsLayer::~GraphicsLayer() {
            for (auto &i: _windows) {
                i.reset();
            }
            glfwTerminate();
        }
        
        Window& GraphicsLayer::window(int width, int height, bool visible, Window *parent) {
            _windows.push_back(std::unique_ptr<Window>(new Window(width, height, visible, parent)));
            auto &window = *_windows.back().get();
            
            
            if (visible) { // interaction between graphics layer and event layer
                event().registerWindowCallbacks(window);
            }
            
            //        if (visible) {
            //            glfwSetWindowSizeCallback(window, window_size_callback);
            //            glfwSetKeyCallback (window, key_callback);
            //            glfwSetCursorPosCallback (window, cursor_callback);
            //            glfwSetMouseButtonCallback(window, mouse_button_callback);
            //            glfwSetScrollCallback(window, wheel_callback);
            //        }
            
            return window;
        }
        
        Window& GraphicsLayer::window(WindowHandle handle) const {
            for (auto &i: _windows) {
                if (i.get()->handle == handle)
                    return *i.get();
            }
            throw std::runtime_error("no glcontext with given window");
        }
        
        
        //--------------------------------------------------------------------------
        // EventsLayer
        //--------------------------------------------------------------------------
        
        void window_size_callback(GLFWwindow* window, int width, int height);
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void cursor_callback(GLFWwindow *window, double x, double y);
        void wheel_callback(GLFWwindow *window, double x, double y);
        void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
        
        EventLayer& EventLayer::registerWindowCallbacks(const Window& window) {
            auto glfw_window = glfwwin(window.handle);
            glfwSetWindowSizeCallback(glfw_window, window_size_callback);
            glfwSetKeyCallback (glfw_window, key_callback);
            glfwSetCursorPosCallback (glfw_window, cursor_callback);
            glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
            glfwSetScrollCallback(glfw_window, wheel_callback);
            return *this;
        }
        
        EventLayer& EventLayer::callback(EventCallbackType cb) {
            _callback = cb;
            return *this;
        }
        
        EventLayer& EventLayer::trigger(event::Event *e) {
            if (_callback)
                _callback(e);
            return *this;
        }
        
        EventLayer& EventLayer::poll() {
            glfwPollEvents();
            return *this;
        }
        
        //------------------------------------------------------------------------------
        // free function
        //------------------------------------------------------------------------------
        
        GraphicsLayer& graphics() {
            static GraphicsLayer graphics_layer;
            return graphics_layer;
        }
        
        EventLayer& event() {
            static EventLayer event_layer;
            return event_layer;
        }
        
        
        
        //------------------------------------------------------------------------------
        // Callbacks
        //------------------------------------------------------------------------------
        
        void window_size_callback(GLFWwindow* glfwwindow, int width, int height)
        {
            auto &window = graphics().window(handle(glfwwindow));
            
            //            int actual_width, actual_height;
            //            glfwGetWindowSize( window, &actual_width, &actual_height);
            
            window.width  = width;
            window.height = height;
            
            window.framebuffer_width  = width  * window.window_to_framebuffer_factor;
            window.framebuffer_height = height * window.window_to_framebuffer_factor;
            
            event::WindowResize resize_event ({ (double) width, (double) height });
            
            event().trigger(&resize_event);
        }
        
        void key_callback(GLFWwindow* glfwwindow, int key, int scancode, int action, int mods)
        {
            // auto &window = graphics().window(handle(glfwwindow));
            
            // don't bother with repeat
            if (action == GLFW_REPEAT)
                return;
            
            event::Modifiers modifiers(GLFW_MOD_SHIFT   & mods,
                                       GLFW_MOD_CONTROL & mods,
                                       GLFW_MOD_ALT     & mods);
            
            
            auto key_code = (event::KeyCode) key;
            
            if (action == GLFW_PRESS) {
                event::KeyPress key_press { key_code, modifiers };
                event().trigger(&key_press);
            }
            else {
                event::KeyRelease key_release { key_code, modifiers };
                event().trigger(&key_release);
            }
            //
            //        auto e_type = action == GLFW_PRESS ? event::EVENT_KEY_PRESS : event::EVENT_KEY_RELEASE;
            //
            //
            //        event::Ke
            //
            //        if (e_type == event::EVENT_MOUSE_PRESS) {
            //            event::KeyEvent e(e_type, key_code, modifiers);
            //
            //        }
            //        else if (e_type == event::EVENT_KEY_RELEASE) {
            //
            //        }
            //
            //        main_instance.signal_key.trigger(opengl_context, e);
        }
        
        void cursor_callback(GLFWwindow *glfwwindow, double x, double y) {
            auto &window = graphics().window(handle(glfwwindow));
            
            // x and y comes in framebuffer coords
            //            x /= opengl_context.window_to_framebuffer_factor;
            //            y /= opengl_context.window_to_framebuffer_factor;
            
            event::MouseMove mouse_move ( { std::floor(x), window.height - 1 - std::floor(y) } );
            //
            //        event::MouseEvent e({ (int) x, (int) opengl_context.height - 1 - (int) y });
            //
            //        main_instance.signal_mouse_move.trigger(opengl_context, e);
            
            event().trigger(&mouse_move);
        }
        
        void wheel_callback(GLFWwindow *glfwwindow, double x, double y) {
            
            // auto &window = graphics().window(handle(glfwwindow));
            
            //        double xx, yy;
            //        glfwGetCursorPos(glfwwindow, &xx, &yy);
            
            // x and y comes in framebuffer coords
            //            xx /= opengl_context.window_to_framebuffer_factor;
            //            yy /= opengl_context.window_to_framebuffer_factor;
            
            //        Point pos { xx, window.height - 1 - yy};
            event::Modifiers modifiers;
            
            
            Point delta {
                (x > 0 ? 1.0 : (x < 0 ? -1.0 : 0.0)),
                (y > 0 ? 1.0 : (y < 0 ? -1.0 : 0.0)) };
            event::MouseWheel wheel_event { delta, modifiers };
            
            event().trigger(&wheel_event);
            // event::WheelEvent e(event::WHEEL_EVENT, modifiers, pos, delta);
            // main_instance.signal_wheel.trigger(opengl_context, e);
            
        }
        
        
        void mouse_button_callback(GLFWwindow *glfwwindow, int button, int action, int mods) {
            
            // auto &window = graphics().window(handle(glfwwindow));

            event::Modifiers modifiers(GLFW_MOD_SHIFT   & mods,
                                       GLFW_MOD_CONTROL & mods,
                                       GLFW_MOD_ALT     & mods);

            auto btn  = (button == GLFW_MOUSE_BUTTON_LEFT) ?
            event::MOUSE_BUTTON_LEFT : ((button == GLFW_MOUSE_BUTTON_RIGHT) ? event::MOUSE_BUTTON_RIGHT :
                                        event::MOUSE_BUTTON_MIDDLE);

            if (action == GLFW_PRESS) {
                event::MousePress mouse_press { btn, modifiers };
                event().trigger(&mouse_press);
            }
            else {
                event::MouseRelease mouse_release { btn, modifiers };
                event().trigger(&mouse_release);
            }
            
//            auto e_type = (action == GLFW_PRESS) ? event::MOUSE_PRESS_EVENT : event::;
//            auto e_btn  = (button == GLFW_MOUSE_BUTTON_LEFT) ?
//            event::mouse::LeftButton : ((button == GLFW_MOUSE_BUTTON_RIGHT) ? event::mouse::RightButton :
//                                        event::mouse::MiddleButton);
//            
//            
//            event::MouseEvent e(e_type, e_btn, modifiers);
//            
//            double x, y;
//            glfwGetCursorPos(window, &x, &y);
//            
//            // x and y comes in framebuffer coords
//            //            x /= opengl_context.window_to_framebuffer_factor;
//            //            y /= opengl_context.window_to_framebuffer_factor;
//            
//            e.position = { (int) x, (int) opengl_context.height - 1 - (int) y };
//            
//            
//            main_instance.signal_mouse_button.trigger(opengl_context, e);
        }
        
    } // namespace lluitk
    
} // namespace os
