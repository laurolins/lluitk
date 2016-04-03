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

        
        struct Weights {
            Weights() = default;
            Weights(const Vec2 &f, const Vec2& v): fixed(f), variable(v), empty(false) {}
            operator bool() const { return !empty; }
            Vec2 variable;
            Vec2 fixed;
            bool empty { true };
            
        };
        
        
        Weights merge_weights(const Weights &w0, const Weights &w1, DivisionType dt) {
            assert(w0 && w1);
            Weights result;
            if (dt == HORIZONTAL) {
                // linear on the horizontal
                result.empty = false;
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
                result.empty = false;
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
            std::function<Weights(Node*)> get_weights;
            std::function<void(Node*,const Vec2&, const Vec2&)> update_window;
            
            auto that = this;
            
            // updates the variable division weights for a 2nd pass
            // fixing the window position and size of all slots and
            // (separation bars)
            get_weights = [&get_weights,that](Node* node) {
                if (!node->visible()) {
                    return Weights(); // empty
                }
                else if (node->is_division()) {
                    auto division = node->as_division();

                    auto node0 = division->get(0);
                    auto node1 = division->get(1);
                    
                    assert(node0 && node1 && "Invariant not being obeyed: every division has two slots");

                    auto w0 = get_weights(node0);
                    auto w1 = get_weights(node1);
                    
                    if (w0 && w1) {
                        auto w = merge_weights(w0,w1,division->type());
                        w.fixed = w.fixed + (division->type() == HORIZONTAL ? Vec2(that->border_size,0) : Vec2(0,that->border_size));
                        node->weight(w.variable);
                        return w;
                    }
                    else if (w0) { node->weight(w0.variable); return w0; }
                    else if (w1) { node->weight(w1.variable); return w1; }
                    else { node->weight(Vec2(0,0)); return Weights(); }
                }
                else if (node->is_slot()) {
                    return Weights(Vec2(0,0),node->weight());
                }
                assert(0 && "invalid case!");
            };
            
            
            Weights weights;
            if (_root) {
                weights = get_weights(_root.get());
            }
            
            std::cout << "fix: " << weights.fixed << std::endl;
            std::cout << "var: " << weights.variable << std::endl;
            
            double xcoef = (window.width()  - weights.fixed.x() - 2*margin_size) / weights.variable.x();
            double ycoef = (window.height() - weights.fixed.y() - 2*margin_size) / weights.variable.y();
            
            // let's say everything fits together
            
            update_window = [&update_window,xcoef,ycoef,that](Node* node, const Vec2& pos, const Vec2& scale) {
                if (!node->visible()) { node->window(Window(0,0,0,0)); }
                if (node->is_slot()) {
                    node->window(Window(pos.x(),
                                        pos.y(),
                                        xcoef * node->weight().x() * scale.x(),
                                        ycoef * node->weight().y() * scale.y()));
                }
                else if (node->is_division()) {
                    auto division = node->as_division();
   
                    auto dt = division->type();
                    
                    auto node0 = division->get(0);
                    auto node1 = division->get(1);

                    // set mid rectangle as the window of a division
                    if (dt == HORIZONTAL) {
                        
                        // expand unbalanced weights
                        auto h0 = node0->weight().y();
                        auto h1 = node1->weight().y();
                        auto scale0 = Vec2(scale.x(), scale.y() * ((h0 < h1) ? h1/h0 : 1.0) );
                        auto scale1 = Vec2(scale.x(), scale.y() * ((h1 < h0) ? h0/h1 : 1.0) );
                        
                        update_window(node0,pos,scale0);
                        node->window(Window(node0->window().X(),pos.y(),that->border_size,node0->window().height()));
                        update_window(node1,node->window().Xy(),scale1);
                    }
                    else { // vertical dt

                        // expand unbalanced weights
                        auto w0 = node0->weight().x();
                        auto w1 = node1->weight().x();
                        auto scale0 = Vec2(scale.x() * ((w0 < w1) ? w1/w0 : 1.0), scale.y());
                        auto scale1 = Vec2(scale.x() * ((w1 < w0) ? w0/w1 : 1.0), scale.y());
                        
                        update_window(node1,pos,scale1);
                        node->window(Window(pos.x(),node1->window().Y(),node1->window().width(),that->border_size));
                        update_window(node0,node->window().xY(),scale0);
                    }
                }
            };
            
            update_window(_root.get(),window.xy() + Vec2(margin_size),{1.0,1.0});
            
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

