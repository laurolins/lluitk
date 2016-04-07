#pragma once

#include <cassert>

#include "simple_widget.hh"
#include "canvas.hh"

#include "llsg/llsg.hh"
#include "llsg/llsg_opengl.hh"

#include <vector>

namespace lluitk {
    
    namespace grid2 {
        
        struct Node;
        struct Slot;
        struct Division;
        
        //
        // Avoiding inheritance: explicit encoding of a inheritance simulation
        // virtual pointer to a place where the appropriate methods should be
        //
        
        struct NodeUniquePtr {
        public:
            Node *_node { nullptr };
            
            
        public:
            NodeUniquePtr() = default;

            ~NodeUniquePtr();
            
            NodeUniquePtr(const NodeUniquePtr& other) = delete;
            NodeUniquePtr& operator=(const NodeUniquePtr& other) = delete;

            NodeUniquePtr(NodeUniquePtr&& other) { std::swap(_node, other._node); }
            NodeUniquePtr& operator=(NodeUniquePtr&& other) {  std::swap(_node, other._node); return *this; }
            
            void reset() { this->~NodeUniquePtr(); _node = nullptr; }
            
            void reset(Node *node) { this->~NodeUniquePtr(); _node = node; }
//            void reset(Slot *slot) { this->~NodeUniquePtr(); _node = (Node*) slot; }
//            void reset(Division *division) { this->~NodeUniquePtr(); _node = (Node*) division; }

            Node* release() { auto result=_node; _node = nullptr; return result; }
            
            operator bool() const { return _node != nullptr; }
            
            Node* get() { return _node; }
            const Node* get() const{ return _node; }

            Node* operator*() { return _node; }
            const Node* operator*() const { return _node; }
            
            Node* operator->() { return _node; }
            const Node* operator->() const { return _node; }
            
        };
        

        enum NodeType     { SLOT, DIVISION       };
        enum DivisionType { HORIZONTAL, VERTICAL };
        

        //---------
        // Weights
        //---------
        
        struct Weights {
            Weights() = default;
            Weights(const Vec2 &f, const Vec2& v): fixed(f), variable(v) {}
            Vec2 variable;
            Vec2 fixed;
        };
        
        //-------
        // Node
        //-------
        
        struct Node {
        public: // data members
            Division* _parent { nullptr }; // doesn't own this pointer
            int       _index { -1 };       // index on parent's list
            uint32_t  _status { 0x1};      // visible, resizeable, hittable, etc, etc...

            Window    _window;             // once not dirty, window of this slot

            Weights   _weights;            // slot weights are user defined, Division weights are computed

            NodeType  _node_type;

        public:

            Node(NodeType node_type): _node_type(node_type){}
            Node(Division* parent, NodeType node_type, int index):
            _parent(parent),_node_type(node_type), _index(index)
            {}
            
            Window window() const { return _window; }
            void window(const Window& w) { _window = w; }
            
            bool   visible() const { return _status & 0x1; }
            void   visible(bool f) { _status = f ? 0x1 : 0; }

            Division* parent() { return _parent; }
            const Division* parent() const { return _parent; }
            void parent(Division* d) { _parent = d; }

            NodeType node_type() const { return _node_type; }
            void node_type(NodeType t) { _node_type = t; }
            
            bool is_slot() const { return _node_type == SLOT; }
            bool is_division() const { return _node_type == DIVISION; }
            
            Slot*     as_slot()     { assert(_node_type == SLOT);     return (Slot*) this;     }
            Division* as_division() { assert(_node_type == DIVISION); return (Division*) this; }

            // returns the new division (plugged to its parent and everything)
            Division* split(DivisionType d);
            
            int index() const { return _index; }
            void index(int index) { _index = index; }
        
            const Weights& weights() const { return _weights; }
            Weights& weights() { return _weights; }
            void     weights(const Weights& w) { _weights=w; }
        };
        
        //-------
        // Slot
        //-------
        
        struct Slot {
        public:
            Node      _node; // assumes it starts with Node
            Widget*   _widget { nullptr }; // actual content of this slot
            
            // a number a user defined; can be used to restore saved layouts
            int       _user_number { -1 };
            
        public:
            Slot(): _node(SLOT) {}
            
            Node* node() { return &_node; }
            Node& as_node() { return _node; }

            Widget* widget() const { return _widget; }
            void widget(Widget* w) { _widget = w; }
            
            int user_number() const { return _user_number; }
            void user_number(int user_number) { _user_number = user_number; }

        };
        
        //-----------
        // Division
        //-----------
        
        struct Division {
        public:
            Node   _node; // starts as a Node
            NodeUniquePtr _children[2]; // two
            DivisionType  _type { HORIZONTAL };
        public:
            Division():_node(DIVISION) {}
            
            Node& as_node() { return _node; }
            Node* node() { return &_node; }
            
            Node* get(int index)  { assert(index>=0 && index<2); return _children[index].get(); }
            const Node* get(int index) const { assert(index>=0 && index<2); return _children[index].get(); }

            // overwrites current node (reclaim all memory
            // downstream of the current node at "index").
            void  set(int index, Node* n);

            // release child without destroying it,
            // not managing its lifecycle any more
            void  release(int index);

            DivisionType type() const { return _type; }
            void type(DivisionType t) { _type=t; }
            
            Window separator_window() const;
            
            Node* other(Node *node);
            
        };
        
        //-------
        // Grid2
        //-------
        
        struct Grid2: public lluitk::SimpleWidget {
        public:
            NodeUniquePtr _root; // has to delete node on destruction
            bool _dirty; // one some node becomes visible/invisible or some

            // weight grows, there should be a recalculation of
            // the slot sizes
            Window _window;
            
            // when things don't fit, what do we do?
            int _border_size { 5 }; // { 10 }; // division size
            int _margin_size { 5 }; // { 5 }; // outsize margin
            
            // draw invisible shapes to quickly figure
            // mouse events
            llsg::Group _scene_root;
            bool        _resizing { false };
            Division*   _resizing_division { nullptr };
            Vec2        _resizing_weight_per_pixel;
            
        public:
            Grid2() { _scene_root.style().color().reset({1.0f}); }
            
            bool dirty() const { return _dirty; }
            void dirty(bool flag) { _dirty = flag; }
            
            Node* root() { return _root.get(); }
            const Node* root() const { return _root.get(); }

            int border_size() const { return _border_size; }
            int margin_size() const { return _margin_size; }

            void border_size(int b) { _border_size = b; dirty(true); }
            void margin_size(int m) { _margin_size = m; dirty(true); }

            // return the slot
            Slot* insert(Widget *widget, int user_number=-1, Node* at=nullptr, DivisionType dt=HORIZONTAL);
            void remove(Node* node);

            void remove_and_simplify(Node* node);

            void window(const Window& w) { _window=w; dirty(true); }
            const Window& window() const { return _window; }
            
            void render();
            
            void swap_widgets(Slot *s1, Slot *s2) { auto aux = s1->widget(); s1->widget(s2->widget()); s2->widget(aux); }
            
            // compute window sizes of all slots
            void update();

            //
            // a division which is part of a chain of
            // same type divisions will be rotated to the
            // bottom of the chain (example usage: local
            // rotation of areas)
            //
            Division* localize_division(Division *d);

            //
            // ascii string representation of the grid's current states
            // returns the code size (if greater than buffer_size, the
            // filled buffer is incomplete). Avoid allocation
            // responsibilities
            //
            int code(char *buffer=nullptr, int buffer_size=0);
            
            //
            // null terminated buffer with the code for which we
            // want to use to initialize the grid, all previous
            // grid info will be erased
            //
            // void reset(const char* code);
            
        public:
            
            void onMousePress(const lluitk::App &app);
            void onMouseMove(const lluitk::App &app);
            void onMouseRelease(const lluitk::App &app);

        public:
            
            bool contains(const Point& p) const { return _window.contains(p); }
            
            WidgetIterator children() const;
            WidgetIterator reverse_children() const;
            
            void sizeHint(const Window &window);

        };

        //--------------
        // NodeIterator
        //--------------
        
        struct NodeIterator {
            NodeIterator() = default;
            NodeIterator(Node *n) { if (n) _stack.push_back(n); }
            Node* next();
            std::vector<Node*> _stack;
        };

        
        //----------------
        // WidgetIterator
        //----------------
        
        struct NodeWidgetIterator: public BaseWidgetIterator {
            NodeWidgetIterator() = default;
            NodeWidgetIterator(Node *n): _iter(n) {}
            Widget* next();
            NodeIterator _iter;
        };
        
        
        //-------------------------
        // Generate grid from code
        //-------------------------
        
        //
        // not that all slots will have no widget pointer
        // it is just the layout that is recovered as well
        // as user_numbers for the slots
        //
        int parse(const char *code, Grid2& output);


        
    } // grid2
    
} // lluitk