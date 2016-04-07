#include "grid2.hh"

#include "app.hh"

#include "llsg/llsg_opengl.hh"

#include <sstream>

namespace lluitk {
    
    namespace grid2 {
        
        //------
        // Node
        //------
        
        NodeUniquePtr::~NodeUniquePtr() {
            if (_node) { // release memory in the right way before deallocation
                // std::cout << "deleting node: " << (void*) this << std::endl;
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
        
        Window Division::separator_window() const {
            if (_type == HORIZONTAL) {
                return Window(get(0)->window().Xy(), get(1)->window().xY());
            }
            else {
                return Window(get(1)->window().xY(), get(0)->window().Xy());
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
        
        Node* Division::other(Node *node) {
            if (_children[0].get() == node) { return _children[1].get(); }
            else if (_children[1].get() == node) { return _children[0].get(); }
            assert(0 && "not matching assumptions of Division::other");
            return nullptr;
        }
        
        //---------------------
        // ExtremeNodeIterator
        //---------------------
        
        struct ExtremeSlotIterator {
            ExtremeSlotIterator(Node *n, DivisionType dt, int direction): _type(dt), _direction(direction) {
                if (n) _stack.push_back(n);
            }
            
            Slot* next() {
                if (!_stack.size()) return nullptr;
                auto node = _stack.back();
                _stack.pop_back();
                while (node->is_division()) {
                    auto division = node->as_division();
                    if (division->type() == _type) {
                        _stack.push_back(division->get(_direction));
                    }
                    else {
                        _stack.push_back(division->get(0));
                        _stack.push_back(division->get(1));
                    }
                    node = _stack.back();
                    _stack.pop_back();
                }
                assert(node->is_slot());
                return node->as_slot();
            }
            
            DivisionType _type;
            int _direction; // index
            std::vector<Node*> _stack;
        };
        

        
        //-------
        // Grid2
        //-------
        
        Slot* Grid2::insert(Widget* w, int user_number, Node* at, DivisionType dt) {
            Slot* new_slot = new Slot();
            new_slot->widget(w);
            new_slot->user_number(user_number);
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
            dirty(true);
        }

        void Grid2::remove_and_simplify(Node* node) {
            assert(node && _root && "Grid::remove problem!");
            if (node == _root.get()) { // it is the root?
            
                _root.reset(); // clear everything
            
            }
            else { // parent exists

                assert(node->parent() && "Grid::remove problem!");
                
                // overwrite with nullptr
                // triggers the release of node
                auto parent       = node->parent();
                auto parent_index = parent->as_node().index();

                auto sibling      = parent->other(node);
                assert(sibling && "not matching assumption of remove_and_simplify");

                parent->release(sibling->index()); // keep reference to sibling (will replace parent)

                if (parent_index >= 0) { // ther eis a grandparent
                    auto parent_index = parent->as_node().index();
                    if (parent_index >= 0) {
                        auto grandpa = parent->as_node().parent();
                        grandpa->release(parent_index);
                        grandpa->set(parent_index, sibling); // should erase parent
                    }
                    else {
                        _root.reset(sibling);
                    }
                }
            }
            dirty(true);
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
        void Grid2::update() {
            
            if (!dirty()) return;
            
            auto window = _window;
            
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
                    w.fixed = w.fixed + (division->type() == HORIZONTAL ? Vec2(that->border_size(),0) : Vec2(0,that->border_size()));
                    node->weights(w);
                }
            };
            
            
            if (_root) { update_weights(_root.get()); }
            
            update_window = [&update_window,that](Node* node, const Window& area) {
                node->window(area);
                if (node->is_division()) {
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
                        
                        update_window (node0, Window(area.x(),            area.y(), w0,                 area.height()));
                        update_window (node1, Window(area.X() - w1,       area.y(), w1,                 area.height()));
                    }
                    else { // vertical dt
                        auto h  = area.height();
                        auto ycoef = (h - node->weights().fixed.y()) / node->weights().variable.y();
                        auto h0 = node0->weights().fixed.y() + node0->weights().variable.y() * ycoef;
                        auto h1 = node1->weights().fixed.y() + node1->weights().variable.y() * ycoef;
                        
                        update_window ( node1, Window(area.x(), area.y(),            area.width(), h1                ));
                        update_window ( node0, Window(area.x(), area.Y() - h0,       area.width(), h0                ));
                    }
                }
            };
            
            update_window(_root.get(),Window(window.xy() + Vec2(margin_size()), window.XY() - Vec2(margin_size())));
            
            { // update hotspots
                _scene_root.removeAll();
                Node* node;
                NodeIterator it(_root.get());
                while ((node = it.next())) {
                    if (node->is_slot()) {
                        _scene_root.rect()
                        .rect(node->window())
                        .visible(false)
                        .hittable(true)
                        .data(node);
                    }
                    else {
                        _scene_root.rect()
                        .rect(node->as_division()->separator_window())
                        .visible(false)
                        .hittable(true)
                        .data(node);
                    }
                }
            }
            
            dirty(false);
            
        }
        
        Division* Grid2::localize_division(Division *d) {
            
            auto dt = d->type();
            
            Division* side0_prev = d;
            Node*     side0_node = d->get(0);
            while (side0_node->is_division() && side0_node->as_division()->type() == dt) {
                side0_prev = side0_node->as_division();
                side0_node = side0_prev->get(1);
            }

            Division* side1_prev = d;
            Node*     side1_node = d->get(1);
            while (side1_node->is_division() && side1_node->as_division()->type() == dt) {
                side1_prev = side1_node->as_division();
                side1_node = side1_prev->get(0);
            }
            
            dirty(true);
            
            // (I) case (0,0): it is already a local division nothing needs to be done
            if (side0_prev == d && side1_prev == d) {
                // std::cout << "case I" << std::endl;
                dirty(false); return d;
            }
            
            // (II) case (1+,1+) there will be a rotation
            else if (side0_prev != d && side1_prev !=d) {
                
                // std::cout << "case II" << std::endl;

                auto beta = side1_prev->get(1);

                side0_prev->release(1); // side0_node
                side1_prev->release(0); // side1_node
                side1_prev->release(1); // beta
                
                // release side1_prev from its parent
                auto side1_prev_parent = side1_prev->node()->parent();
                auto side1_prev_index  = side1_prev->node()->index();
                side1_prev_parent->release(side1_prev_index); // pps1
                
                // reattach things
                side1_prev_parent->set(side1_prev_index,beta);

                side0_prev->set(1, side1_prev->node());
                
                side1_prev->set(0, side0_node);
                side1_prev->set(1, side1_node);

                return side1_prev;
            }
            
            // (III) case (1+,0) there will be a rotation
            else if (side1_prev == d) {
                
                // std::cout << "case III" << std::endl;
                
                auto alpha = side0_prev->get(0);
                side0_prev->release(0);
                
                // release side0_prev from its parent
                auto side0_prev_parent = side0_prev->node()->parent();
                auto side0_prev_index  = side0_prev->node()->index();
                side0_prev_parent->release(side0_prev_index);
                
                side0_prev->release(1);
                side0_prev->set(0, side0_node);
                
                side0_prev_parent->set(side0_prev_index, alpha);
                
                side1_prev->release(1);
                side1_prev->set(1, side0_prev->node());

                side0_prev->set(1, side1_node);
                return side0_prev;
            }
            
            // (IV) case (0,1+) there will be a rotation
            else  { // (side1_prev == d)

                // std::cout << "case IV" << std::endl;

                auto beta = side1_prev->get(1);
                side1_prev->release(1);
                
                // release side0_prev from its parent
                auto side1_prev_parent = side1_prev->node()->parent();
                auto side1_prev_index  = side1_prev->node()->index();
                side1_prev_parent->release(side1_prev_index);
                
                side1_prev->release(0);
                side1_prev->set(1, side1_node);
                
                side1_prev_parent->set(side1_prev_index, beta);
                
                side0_prev->release(0);
                side0_prev->set(0, side1_prev->node());

                side1_prev->set(0, side0_node);
                return side1_prev;
            }
        }

        WidgetIterator Grid2::children()         const { return WidgetIterator(new NodeWidgetIterator(const_cast<Node*>(_root.get()))); }
        
        WidgetIterator Grid2::reverse_children() const { return WidgetIterator(new NodeWidgetIterator(const_cast<Node*>(_root.get()))); }
        
        void Grid2::sizeHint(const Window &window) {
            this->window(window);
            this->update();
            NodeIterator it(_root.get());
            Node* node;
            while ((node = it.next())) {
                if (node->is_slot() && node->as_slot()->widget()) {
                    auto w = node->window();
                    node->as_slot()->widget()->sizeHint(Window(Vec2(std::round(w.x()), std::round(w.y())),
                                                               Vec2(std::round(w.X()), std::round(w.Y()))));
                }
            }
        }
        
        void Grid2::render() {
            auto it = children();
            Widget *w;
            // int count = 0;
            while ((w=it.next())) {
                // sstd::cout << count++ << std::endl;
                w->render();
            }
            
            //
            // draw invisible handles for event detection
            //
            if (dirty()) {
                this->update();
            }
            
            //
            // for debugging location of handles
            // llsg::opengl::getRenderer().render(_scene_root, llsg::Transform(), _window, false);
            //
        }
        
        void Grid2::onMousePress(const lluitk::App &app) {

//            if (!(app.current_event_info.modifiers.shift && app.current_event_info.modifiers.alt && app.current_event_info.modifiers.control))
//                return;
            
            auto pos = app.current_event_info.mouse_position;
            llsg::GeometricTests g;
            auto e = g.firstHit(llsg::Vec2{(double)pos.x(), (double)pos.y()}, _scene_root);
            
            if (e && any::can_cast<Node*>(e->data())) {
                auto node = any::any_cast<Node*>(e->data());
                if (node->is_division()) {
                    auto division = node->as_division();
                    if (app.current_event_info.left_button()) {
                        auto division = node->as_division();
                        _resizing = true;
                        _resizing_division = division;
                        // how much is a pixel worth?
                        if (division->type() == HORIZONTAL) {
                            auto vx = node->weights().variable.x();
                            auto fx = node->weights().fixed.x();
                            auto wx  = node->window().width();
                            auto weight_per_pixel = vx / (wx - fx);
                            _resizing_weight_per_pixel = Vec2(weight_per_pixel, 0.0);
                        }
                        else if (division->type() == VERTICAL) {
                            auto vy = node->weights().variable.y();
                            auto fy = node->weights().fixed.y();
                            auto wy  = node->window().height();
                            auto weight_per_pixel = vy / (wy - fy);
                            _resizing_weight_per_pixel = Vec2(0.0, weight_per_pixel);
                        }
                        app.lock(this);
                        app.finishEventProcessing();
                    }
                    else {
                        auto dt = node->as_division()->type();
                        if (app.current_event_info.modifiers.shift) {
                            auto localized_division = this->localize_division(division);
                            localized_division->type(dt == HORIZONTAL ? VERTICAL : HORIZONTAL);
                            sizeHint(_window);
                            app.finishEventProcessing();
                        }
                        else if (app.current_event_info.modifiers.control) {
                            division->type(dt == HORIZONTAL ? VERTICAL : HORIZONTAL);
                            sizeHint(_window);
                            app.finishEventProcessing();
                        }
                        else {
                            auto dt = division->type();
                            
                            Division* side0_prev = division;
                            Node*     side0_node = division->get(0);
                            while (side0_node->is_division() && side0_node->as_division()->type() == dt) {
                                side0_prev = side0_node->as_division();
                                side0_node = side0_prev->get(1);
                            }
                            
                            Division* side1_prev = division;
                            Node*     side1_node = division->get(1);
                            while (side1_node->is_division() && side1_node->as_division()->type() == dt) {
                                side1_prev = side1_node->as_division();
                                side1_node = side1_prev->get(0);
                            }
                            
                            if (division != side0_prev) { side0_prev->release(1); } else { division->release(0); }
                            if (division != side1_prev) { side1_prev->release(0); } else { division->release(1); }
                            if (division != side0_prev) { side0_prev->set(1,side1_node); } else { division->set(0,side1_node); }
                            if (division != side1_prev) { side1_prev->set(0,side0_node); } else { division->set(1,side0_node); }
                            this->sizeHint(_window);
                            app.finishEventProcessing();
                        }
                    }
                }
            }
        }
        
        
        
        void Grid2::onMouseMove(const lluitk::App &app) {
            if (_resizing) {
                auto delta_px = app.current_event_info.mouse_position - app.last_event_info.mouse_position;
                
                
                // to be applied on the first subtree
                Vec2 delta_weight(_resizing_weight_per_pixel.x() * delta_px.x(),
                                  _resizing_weight_per_pixel.y() * delta_px.y());
                
                
                
                
                
                auto dt = _resizing_division->type();
                
                // figure out the minimum current weight in the negative direction
                double min_weight = 1e50;
                const double EPSILON = 1e-6;
                if (dt == HORIZONTAL) {
                    if (delta_weight.x() < 0) {
                        ExtremeSlotIterator it(_resizing_division->get(0), HORIZONTAL, 1);
                        Slot* slot; while ((slot=it.next())) { min_weight = std::min(slot->node()->weights().variable.x(),min_weight); }
                        if (min_weight + delta_weight.x() < EPSILON) delta_weight.x(0.0);
                    } else {
                        ExtremeSlotIterator it(_resizing_division->get(1), HORIZONTAL, 0);
                        Slot* slot; while ((slot=it.next())) { min_weight = std::min(slot->node()->weights().variable.x(),min_weight); }
                        delta_weight.x(std::max(-min_weight, delta_weight.x()));
                        if (min_weight - delta_weight.x() < EPSILON) delta_weight.x(0.0);
                    }
//                    std::cout << min_weight << std::endl;
                }
                else { // (dt == VERTICAL)
                    if (delta_weight.y() < 0) {
                        ExtremeSlotIterator it(_resizing_division->get(1), VERTICAL, 0);
                        Slot* slot; while ((slot=it.next())) { min_weight = std::min(slot->node()->weights().variable.y(),min_weight); }
                        if (min_weight + delta_weight.y() < EPSILON) delta_weight.y(0.0);
                    } else {
                        ExtremeSlotIterator it(_resizing_division->get(0), VERTICAL, 1);
                        Slot* slot; while ((slot=it.next())) { min_weight = std::min(slot->node()->weights().variable.y(),min_weight); }
                        if (min_weight - delta_weight.y() < EPSILON) delta_weight.y(0.0);
                    }
                }


                // apply weight change
                if (dt == HORIZONTAL) {
                    {
                        ExtremeSlotIterator it(_resizing_division->get(0), HORIZONTAL, 1);
                        Slot* slot; while ((slot=it.next())) { slot->node()->weights().variable.xinc(delta_weight.x()); }
                    }
                    {
                        ExtremeSlotIterator it(_resizing_division->get(1), HORIZONTAL, 0);
                        Slot* slot; while ((slot=it.next())) { slot->node()->weights().variable.xinc(-delta_weight.x()); }
                    }
                    dirty(true);
                    this->sizeHint(_window);
                }
                else { // if (dt == VERTICAL) {
                    {
                        ExtremeSlotIterator it(_resizing_division->get(1), VERTICAL, 0);
                        Slot* slot; while ((slot=it.next())) { slot->node()->weights().variable.yinc(delta_weight.y()); }
                    }
                    {
                        ExtremeSlotIterator it(_resizing_division->get(0), VERTICAL, 1);
                        Slot* slot; while ((slot=it.next())) { slot->node()->weights().variable.yinc(-delta_weight.y()); }
                    }
                    dirty(true);
                    this->sizeHint(_window);
                }
                // std::cout << delta << std::endl;
                app.finishEventProcessing();
            }
        }
        
        void Grid2::onMouseRelease(const lluitk::App &app) {
            if (_resizing) {
                _resizing = false;
                app.lock();
                app.finishEventProcessing();
            }
        }
        
        int Grid2::code(char *buffer, int buffer_size) {
            
            //
            // simple grammar
            //
            // G : "g" <margin> <border> N ( "0" | N )
            // N : ("h"|"v") N N | "s" <x weight> <y weight> <user number>
            //
            
            // the empty grid will be represented by "g border_size margin_size "
            std::stringstream ss;
            ss << (_root ? "g " : "e ") << margin_size() << " " << border_size();
            if (_root) {
                std::vector<Node*> stack;
                stack.push_back(_root.get());
                while (!stack.empty()) {
                    auto node = stack.back();
                    stack.pop_back();
                    if (node->is_division()) {
                        stack.push_back(node->as_division()->get(1));
                        stack.push_back(node->as_division()->get(0));
                        ss << " " << (node->as_division()->type() == HORIZONTAL ? "h" : "v");
                    }
                    else {
                        ss << " s " << node->weights().variable.x() << " " << node->weights().variable.y() << " " << node->as_slot()->user_number();
                    }
                }
            }

            ss.seekp(0, std::ios::end);
            int size = (int) ss.tellp();

            // copy as much as we can to buffer
            if (buffer) {
                ss.read(buffer,buffer_size-1);
                buffer[buffer_size-1] = 0; // make sure it is null terminated
            }
            
            
            return size;
        }
        
        //
        // returns problem code, if one happens
        // otherwise return zero
        //
        int parse(const char *code, Grid2& output) {
            
            struct Token {
                Token() = default;
                Token(int p): begin(p), end(p) {}
                void grow() { ++end; }
                operator bool() { return begin < end; }
                int begin { 0 };
                int end { 0 };
            };
            
            //
            // simple grammar
            //
            // G : "g" <margin> <border> N | "e" <margin> <border>
            // N : ("h"|"v") N N | "s" <x weight> <y weight> <user number>
            //
            int pos = 0;
            auto next_token = [code, &pos]() {
                while (code[pos] && code[pos] == ' ') ++pos;
                Token tok(pos);
                while (code[pos] && code[pos] != ' ') {
                    tok.grow();
                    ++pos;
                }
                return tok;
            };
            
            enum Error { MISSING_TOKENS=1, NUMBER_PROBLEM=2, INVALID_SYNTAX=3, EXTRA_TOKENS=4 };

            // function
            
            Grid2 result;
            
            // generate node
            std::function<int(NodeUniquePtr&)> N;
            
            // fill in grid
            auto G = [&next_token, &result, code, &N]() {
                
                auto tok_kind   = next_token();
                auto tok_margin = next_token();
                auto tok_border = next_token();
                
                if (!tok_kind || !tok_margin || !tok_border)
                    return (int) MISSING_TOKENS;

                try {
                    auto margin = std::stoi(std::string(&code[tok_margin.begin],&code[tok_margin.end]));
                    auto border = std::stoi(std::string(&code[tok_border.begin],&code[tok_border.end]));
                    result.margin_size(margin);
                    result.border_size(border);
                } catch (...) {
                    return (int) NUMBER_PROBLEM;
                }

                auto grid_status = code[tok_kind.begin];
                if (grid_status == 'g') {
                    auto error = N(result._root); // store root on the result
                    if (!error && next_token()) {
                        return (int) EXTRA_TOKENS;
                    }
                    else {
                        return error;
                    }
                }
                else if (grid_status == 'e') {
                    if (next_token()) { return (int) EXTRA_TOKENS; }
                    else { return 0; }
                }
                else { // 'e' empty grid (no node at all)
                    return (int) INVALID_SYNTAX; // done
                }
            };
            
            N = [&code, &next_token, &N](NodeUniquePtr &result) {
                
                auto type = next_token();
                
                if (!type) return (int) MISSING_TOKENS;
                
                auto node_type = code[type.begin];
                if (node_type == 'h' || node_type =='v') { // HORIZONTAL DIVISION
                    result.reset((new Division())->node());
                    result->as_division()->type(node_type == 'h' ? HORIZONTAL : VERTICAL);
                    
                    NodeUniquePtr child_0, child_1;
                    
                    auto error_0 = N(child_0);
                    if (error_0) return error_0;
                    
                    auto error_1 = N(child_1);
                    if (error_1) return error_1;
                    
                    result->as_division()->set(0, child_0.release());
                    result->as_division()->set(1, child_1.release());
                    
                    return 0;
                }
                else if (code[type.begin] == 's') { // SLOT

                    auto tok_xweight     = next_token();
                    auto tok_yweight     = next_token();
                    auto tok_user_number = next_token();
                    
                    result.reset((new Slot())->node());
                    
                    try {
                        auto xweight = std::stof(std::string(&code[tok_xweight.begin],&code[tok_xweight.end]));
                        auto yweight = std::stof(std::string(&code[tok_yweight.begin],&code[tok_yweight.end]));
                        auto user_number = std::stoi(std::string(&code[tok_user_number.begin],&code[tok_user_number.end]));
                        result->weights().variable.x(xweight);
                        result->weights().variable.y(yweight);
                        result->as_slot()->user_number(user_number);
                    } catch (...) {
                        return (int) NUMBER_PROBLEM;
                    }
                    
                    return 0;
                }
                else { return (int) INVALID_SYNTAX; }
            };
            
            auto error = G();
            
            if (!error) { std::swap(result, output); }
            
            return error;
        }

        
        //--------------
        // NodeIterator
        //--------------
        
        Node* NodeIterator::next() {
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
        
        //--------------------
        // NodeWidgetIterator
        //--------------------
        
        Widget* NodeWidgetIterator::next() {
            while (true) {
                auto node = _iter.next();
                if (!node) return nullptr;
                if (node->is_slot() && node->as_slot()->widget()) {
                    return node->as_slot()->widget();
                }
            }
        }
        
    } // grid2

} // lluitk

