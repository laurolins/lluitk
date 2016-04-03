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

            void release() { _node = nullptr; }
            
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
            
        public:
            Slot(): _node(SLOT) {}
            
            Node* node() { return &_node; }
            Node& as_node() { return _node; }

            Widget* widget() const { return _widget; }
            void widget(Widget* w) { _widget = w; }

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
            
            // overwrites current node (reclaim all memory
            // downstream of the current node at "index").
            void  set(int index, Node* n);

            // release child without destroying it,
            // not managing its lifecycle any more
            void  release(int index);

            DivisionType type() const { return _type; }
            void type(DivisionType t) { _type=t; }
            
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
            
        public:
            Grid2() = default;
            
            bool dirty() const { return _dirty; }
            void dirty(bool flag) { _dirty = flag; }

            int border_size() const { return _border_size; }
            int margin_size() const { return _margin_size; }

            void border_size(int b) { _border_size = b; dirty(true); }
            void margin_size(int m) { _margin_size = m; dirty(true); }

            // return the slot
            Slot* insert(Widget *widget, Node* at=nullptr, DivisionType dt=HORIZONTAL);
            void remove(Node* node);
            
            void window(const Window& w) { _window=w; dirty(true); }
            const Window& window() const { return _window; }
            
            void render();
            
            // compute window sizes of all slots
            void update();

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

        
    } // grid2
    
} // lluitk