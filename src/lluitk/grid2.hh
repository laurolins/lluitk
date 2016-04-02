#pragma once

#include "simple_widget.hh"
#include "canvas.hh"

#include "llsg/llsg.hh"
#include "llsg/llsg_opengl.hh"

namespace lluitk {
    
    namespace grid2 {
        
        struct Node;
        struct Slot;
        struct Division;
        
        enum NodeType     { SLOT, DIVISION       };
        enum DivisionType { HORIZONTAL, VERTICAL };
        
        struct Node {
        public: // data members
            NodeType _node_type;
            
        private:
            Node() = default; // cannot be constructed
            
        public:
            NodeType node_type() const { return _node_type; }
            void node_type(NodeType t) { _node_type = t; }
            
            bool is_slot() const { return _node_type == SLOT; }
            bool is_division() const { return _node_type == DIVISION; }
            
            Slot&     as_slot()     { assert(_node_type == SLOT); return *((Slot*) this); }
            Division& as_division() { assert(_node_type == DIVISION); return *((Division*) this); }
        };
        
        struct Slot {
        public:
            Node      _node; // assumes it starts with Node
            float     _weight { 1.0 };
            uint32_t  _status; // visible, resizeable, hittable, etc, etc...
            Widget*   _widget { nullptr }; // actual content of this slot
            Window    _window; // once not dirty, window of this slot
        public:
            Node& as_node() { return _node; }
            float weight()  const { return _weight; }
            Window window() const { return _window; }
            bool   visible() const { return _status & 0x1; }
            bool   visible(bool f) const { return _status = f ? 0x1 : 0; }
        };
        
        struct Division {
        public:
            Node   _node; // starts as a Node
            Slot  *_first { nullptr }
            Slot  *_second { nullptr }
            DivisionType _type { HORIZONTAL };
        public:
            Slot* first() { return _first; }
            Slot* second() { return _second; }
            void first(Slot *s) { _first = s; }
            void second(Slot *s) { _second = s; }
            DivisionType type() const { return _type; }
            void type(DivisionType t) const { _type=t; }
        };
        
        struct Grid2: public lluitk::SimpleWidget {
            
            
            
            
            bool _dirty; // one some node becomes visible/invisible or some
                         // weight grows, there should be a recalculation of
                         // the slot sizes
        };
        
        
        
    } // grid2
    
} // lluitk