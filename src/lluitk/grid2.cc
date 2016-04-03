#include "grid2.hh"


namespace lluitk {
    
    namespace grid2 {
        
        //------
        // Node
        //------
        
        NodeUniquePtr::~NodeUniquePtr() {
            if (_node) { // release memory in the right way before deallocation
                if(_node->is_slot()) { delete _node->as_slot(); }
                else if(_node->is_division()) { delete _node->as_division(); }
                else { assert(0 && "~NodeUniquePtr() not valid!"); }
            }
        }

        //------
        // Node
        //------
        
        Division* Node::split(DivisionType d) {

            auto index = _index;
            
            auto parent = this->parent();
            if (parent) {
                parent->release(_index);
            }
            
            // more explicit code of the wiring
            auto division = new Division();
            division->set(0,this); // set
            division->type(d);
            
            if (parent) {
                parent->set(index, division->node());
            }
            
            return division;
        }
        
        //----------
        // Division
        //----------
        
        void Division::release(int index) {
            assert(index>=0 && index<2);
            if (_children[index]) {
                _children[index].get()->parent(nullptr);
                _children[index].get()->index(-1);
                _children[index].release();
            }
        }
        
        void  Division::set(int index, Node* n)  {
            assert(index>=0 && index<2);
            _children[index].reset(n);
            if (n) {
                n->parent(this);
                n->index(index);
            }
        }
        
        //-------
        // Grid2
        //-------
        
        Slot* Grid2::insert(Widget* w, Node* at, DivisionType dt) {
            Slot* new_slot = new Slot();
            new_slot->widget(w);
            new_slot->node()->weights().variable = Vec2(1.0,1.0);
            
            // if grid is empty, insert into root
            if (!_root) {
                assert(!at && "Grid2::insert problem !_root => !at");
                _root.reset(new_slot->node());
            }
            else {
                if (!at) at = _root.get();
                bool is_root = (at == _root.get());
                
                // check if division has an empty
                if (is_root) _root.release();
                
                auto division = at->split(dt);
                
                if (is_root)_root.reset(division->node());
                
                division->set(1,new_slot->node());
            }

            dirty(true);
            return new_slot;
        }
        
        void Grid2::remove(Node* node) {
            assert(node && _root && "Grid::remove problem!");
            if (node == _root.get()) {
                _root.reset(); // throu away all the data
            }
            else {
                assert(node->parent() && "Grid::remove problem!");
                // overwrite with nullptr
                // triggers the release of node
                node->parent()->set(node->index(), nullptr); // commit suicide... hehe
            }
        }

        
        Weights merge_weights(const Weights &w0, const Weights &w1, DivisionType dt) {
            Weights result;
            if (dt == HORIZONTAL) {
                // linear on the horizontal
                result.variable.x(w0.variable.x() + w1.variable.x());
                result.fixed.x(w0.fixed.x() + w1.fixed.x());
                
                // give priority to the first
                if (w0.variable.y() > w1.variable.y() || (w0.variable.y() == w1.variable.y() && w0.fixed.y() >= w1.fixed.y())) {
                    result.variable.y(w0.variable.y());
                    result.fixed.y(w0.fixed.y());
                }
                else {
                    result.variable.y(w1.variable.y());
                    result.fixed.y(w1.fixed.y());
                }
            }
            else if (dt == VERTICAL) {
                // linear on the horizontal
                result.variable.y(w0.variable.y() + w1.variable.y());
                result.fixed.y(w0.fixed.y() + w1.fixed.y());
                
                // give priority to the first
                if (w0.variable.x() > w1.variable.x() || (w0.variable.x() == w1.variable.x() && w0.fixed.x() >= w1.fixed.x())) {
                    result.variable.x(w0.variable.x());
                    result.fixed.x(w0.fixed.x());
                }
                else {
                    result.variable.x(w1.variable.x());
                    result.fixed.x(w1.fixed.x());
                }
            }
            return result;
        }

        
        // compute window sizes of all slots and
        // divisions (mid-rectangle)
        void Grid2::compute_sizes(const Window& window) {
            
            // compute sum of weights, then linearly distribute through the window
            std::function<void(Node*)> update_weights;
            std::function<void(Node*,const Window&)> update_window;
            
            auto that = this;
            
            // updates the variable division weights for a 2nd pass
            // fixing the window position and size of all slots and
            // (separation bars)
            update_weights = [&update_weights,that](Node* node) {
                if (node->is_division()) {
                    auto division = node->as_division();

                    auto node0 = division->get(0);
                    auto node1 = division->get(1);
                    
                    assert(node0 && node1 && "Invariant not being obeyed: every division has two slots");

                    update_weights(node0); // bottom up weights update
                    update_weights(node1);
                
                    auto w0 = node0->weights();
                    auto w1 = node1->weights();
                    auto w = merge_weights(w0,w1,division->type());
                    w.fixed = w.fixed + (division->type() == HORIZONTAL ? Vec2(that->border_size,0) : Vec2(0,that->border_size));
                    node->weights(w);
                }
            };
            
            
            if (_root) { update_weights(_root.get()); }
            
            update_window = [&update_window,that](Node* node, const Window& area) {
                if (node->is_slot()) {
                    node->window(area);
                }
                else if (node->is_division()) {
                    auto division = node->as_division();
   
                    auto dt = division->type();
                    
                    auto node0 = division->get(0);
                    auto node1 = division->get(1);

                    // set mid rectangle as the window of a division
                    if (dt == HORIZONTAL) {
                        auto w  = area.width();
                        auto xcoef = (w - node->weights().fixed.x()) / node->weights().variable.x();
                        auto w0 = node0->weights().fixed.x() + node0->weights().variable.x() * xcoef;
                        auto w1 = node1->weights().fixed.x() + node1->weights().variable.x() * xcoef;
                        
                        update_window(node0, Window(area.x(),area.y(),w0,area.height()));
                        node->window(Window(node0->window().X(),area.y(),that->border_size,area.height()));
                        update_window(node1, Window(node->window().X(),area.y(),w1,area.height()));
                    }
                    else { // vertical dt
                        auto h  = area.height();
                        auto ycoef = (h - node->weights().fixed.y()) / node->weights().variable.y();
                        auto h0 = node0->weights().fixed.y() + node0->weights().variable.y() * ycoef;
                        auto h1 = node1->weights().fixed.y() + node1->weights().variable.y() * ycoef;
                        
                        update_window(node1, Window(area.x(),area.y(),area.width(),h1));
                        node->window(Window(area.x(),node1->window().Y(),area.width(),that->border_size));
                        update_window(node0, Window(area.x(),node->window().Y(),area.width(),h0));
                    }
                }
            };
            
            update_window(_root.get(),Window(window.xy() + Vec2(margin_size), window.XY() - Vec2(margin_size)));
            
        }

        
        //----------
        // Iterator
        //----------
        
        Node* Iterator::next() {
            if (!_stack.size()) return nullptr;
            auto top = _stack.back();
            _stack.pop_back();
            if (top->is_slot()) { return top; }
            else {
                _stack.push_back(top->as_division()->get(1));
                _stack.push_back(top->as_division()->get(0));
                return top;
            }
        }
        
        
    } // grid2

} // lluitk

