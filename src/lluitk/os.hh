#pragma once

#include <functional>

#include "geom.hh"
#include "event.hh"

namespace lluitk {

/*! \brief objects that interface with lower level operational system stuff (e.g. windows, key/mouse events).
 */
namespace os {

    //------------------------------------------------------------------------------
    // Window Handle
    //------------------------------------------------------------------------------
    
    using WindowHandle = void*;

    //------------------------------------------------------------------------------
    // Window (opengl context)
    //------------------------------------------------------------------------------
    
    struct Window {
    public:

        Window(int width, int height, bool visible=true, Window *parent=nullptr);
        ~Window();
        
        void bind_to_thread();
        void swap_buffers();
        bool done();
        
        void setTitle(const std::string& title);
        
    public:
        WindowHandle   handle            { nullptr };
        Window        *parent            { nullptr };
        bool visible                     { false };
        int width                        { 0 };
        int height                       { 0 };
        int framebuffer_width            { 0 };
        int framebuffer_height           { 0 };
        int window_to_framebuffer_factor { 1 }; // one window pixel == two framebuffer pixels on retina displays
        int requested_height             { 0 };
    };

    //--------------------------------------------------------------------------
    // Graphics Layer
    //--------------------------------------------------------------------------
    
    struct GraphicsLayer {
    public:
        ~GraphicsLayer();
    private:
        GraphicsLayer();
    public:
        Window& window(int width, int height, bool visible=true, Window *parent=nullptr);
        Window& window(WindowHandle handle) const;
    public:
        friend GraphicsLayer& graphics();
    private:
        std::vector<std::unique_ptr<Window>> _windows;
    };

    //--------------------------------------------------------------------------
    // EventsCallbackType
    //--------------------------------------------------------------------------
    
    using EventCallbackType = std::function<void(const ::lluitk::event::Event&)>;
    
    //--------------------------------------------------------------------------
    // EventsLayer
    //--------------------------------------------------------------------------
    
    struct EventLayer {
    public:
        friend struct GraphicsLayer;
        EventLayer& registerWindowCallbacks(const Window& window);
    public:
        EventLayer& poll();
        EventLayer& callback(EventCallbackType cb);
        EventLayer& trigger(const event::Event &e);
    private:
        EventCallbackType _callback;
    };
    
    //--------------------------------------------------------------------------
    // Free Function
    //--------------------------------------------------------------------------
    
    GraphicsLayer& graphics();
    EventLayer&    event();
    
    
    
    
    
    
    
    

//         struct GraphicsLayer {












//         void poll_events();
        

//     public:
//         friend Main& main();

//     public:
//         std::vector<std::unique_ptr<Window>> opengl_contexts;
        
//     public: // signals
        
//         sig::Signal<const OpenGLContext&, int, int>                 signal_resize;
//         sig::Signal<const OpenGLContext&, const event::KeyEvent&>   signal_key;
//         sig::Signal<const OpenGLContext&, const event::MouseEvent&> signal_mouse_move;
//         sig::Signal<const OpenGLContext&, const event::MouseEvent&> signal_mouse_button;
//         sig::Signal<const OpenGLContext&, const event::WheelEvent&> signal_wheel;
    
//     };


















//     struct OS {
//         GraphicsLayer& graphics();
//         EventLayer& events();
//     private:
//         GraphicsLayer _graphics;
//         EventLayer    _events;
//     }

// struct Window {
// }


// //
// // low level implementation is based on glfw,
// // but it should not show in the API
// //


// struct GraphicsLayer {
//     // request windows from the graphics layer 

// };

// struct EventLayer {
//     // poll new input events from event layer (mouse, keyboard)
// }

} // os

} // lluitk
