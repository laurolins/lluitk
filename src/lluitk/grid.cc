#include "grid.hh"

#include <algorithm>
#include <stdexcept>

#include "d3cpp.hh"
#include "app.hh"

namespace lluitk {

    //--------------------------------------------------------------------------
    // GridPoint
    //--------------------------------------------------------------------------
    
    GridPoint::GridPoint(GridLength x, GridLength y):
    _x(x), _y(y)
    {}
    
    GridLength GridPoint::x() const {
        return _x;
        
    }
    GridLength GridPoint::y() const {
        return _y;
    }
    
    GridPoint& GridPoint::x(const GridLength& x) {
        _x = x;
        return *this;
    }

    GridPoint& GridPoint::y(const GridLength& y) {
        _y = y;
        return *this;
    }
    
    GridPoint operator+(const GridPoint& a, const GridPoint &b) {
        return GridPoint { a.x() + b.x(), a.y() + b.y() };
    }

    GridPoint operator-(const GridPoint& a, const GridPoint &b) {
        return GridPoint { a.x() - b.x(), a.y() - b.y() };
    }
    
    bool GridPoint::operator<(const GridPoint& other) const {
        return _x < other._x || (_x == other._x && _y < other._y);
    }
    
    bool GridPoint::operator==(const GridPoint& other) const {
        return _x == other._x && _y == other._y;
    }

    //--------------------------------------------------------------------------
    // Spring
    //--------------------------------------------------------------------------

    Spring::Spring(const Length& length, Type type):
        type(type),
        length(length)
    {}

    Spring& Spring::fixed(const Length& length) {
        type = FIXED_SIZE;
        this->length = length;
        return *this;
    }

    Spring& Spring::weight(const Length& w) {
        type = WEIGHT;
        this->length = w;
        return *this;
    }
    
    Length Spring::weight() const {
        if (type == WEIGHT)
            return this->length;
        throw std::runtime_error("oops");
    }
    
    Length Spring::fixed() const {
        if (type == FIXED_SIZE)
            return this->length;
        throw std::runtime_error("oops");
    }
    
    bool Spring::isFixed() const {
        return type == FIXED_SIZE;
    }
    
    bool Spring::isWeighted() const {
        return type == WEIGHT;
    }

    
    //--------------------------------------------------------------------------
    // Segment
    //--------------------------------------------------------------------------

    Segment::Segment(Type type, int index, const Spring& spring):
        type(type), index(index), _spring(spring)
    {}
    
    Segment& Segment::p0(const Length &len) {
        _p0 = len;
        return *this;
    }
    
    Segment& Segment::size(const Length& len) {
        _size = len;
        return *this;
    }

    Length Segment::p0() const {
        return _p0;
    }
    
    Length Segment::p1() const {
        return _p0 + _size;
    }

    Length Segment::size() const {
        return _size;
    }
    
    Spring& Segment::spring() {
        return _spring;
    }

    const Spring& Segment::spring() const {
        return _spring;
    }

    Segment& Segment::spring(const Spring& spring) {
        _spring = spring;
        return *this;
    }

    
    //--------------------------------------------------------------------------
    // Splitter
    //--------------------------------------------------------------------------
    
    /*! \brief A splitter separates two cells
     */
    Splitter::Splitter(int index, Kind kind):
    index(index), kind(kind)
    {}
    
    bool Splitter::operator==(const Splitter& other) const {
        return index == other.index && kind == other.kind;
    }
    
    bool Splitter::horizontal() const {
        return kind == HORIZONTAL;
    }

    bool Splitter::vertical() const {
        return kind == VERTICAL;
    }
    
    //--------------------------------------------------------------------------
    // Grid
    //--------------------------------------------------------------------------

    Grid::Grid(const GridSize& size) {

        if (size.x() == 0 || size.y() == 0)
            throw std::runtime_error("grid needs at leas one cell");
        
        // create segments with standard sizes
        auto add_segments = [](std::vector<Segment> &segments, int n) {
            segments.push_back(Segment{ Segment::HANDLE,0,Spring().fixed(3)}); // one pixel
            int index = 1;
            for (auto i=0;i<n;++i) {
                segments.push_back(Segment{Segment::CELL,index++,Spring().weight(1.0)}); // one pixel
                segments.push_back(Segment{Segment::HANDLE,index++,Spring().fixed(3)}); // one pixel
            }
        };

        add_segments(horizontal_segments, size.x());
        add_segments(vertical_segments, size.y());
        
    }
    
    void Grid::setCellWidget(const GridPoint& cell, Widget* widget) {
        cell_map[cell] = widget;
    }

    void Grid::sizeHint(const Window &window) {
        this->window = window;
        this->layout();
        
        for (auto &it: cell_map) {
            
            auto &cell  = it.first;
            auto widget = it.second;
            
            auto &hseg = horizontal_segments[1 + 2 * cell.x()];
            auto &vseg = vertical_segments[1 + 2 * cell.y()];
            
            
            auto wmin = window.min() + Point{hseg.p0(), vseg.p0()};
            Window w { wmin, wmin + Point{hseg.size(), vseg.size()} };
            
            widget->sizeHint(w);
        }
        
        canvas.markDirty(true);
    }
    
    
    void Grid::pre_render() {
        
        // some opengl effect must be computed here
        
        auto it = children();
        Widget* child;
        while (it.next(child)) {
            child->pre_render();
        }
    }

    void Grid::render() {
        auto it = children();
        Widget* child;
        while (it.next(child)) {
            child->render();
        }

        //
        // draw the handles after the content
        //
        
        if (canvas.dirty) {
            prepareCanvas();
        }
        auto &renderer = llsg::opengl::getRenderer();
        renderer.render(canvas.root, llsg::Transform(), window);
    }

    Grid& Grid::setInternalHandleFixedSize(int fixed_size) {
        for (auto it=vertical_segments.begin()+1;it!= vertical_segments.end()-1;++it) {
            if (it->type == Segment::HANDLE) {
                it->spring().fixed(fixed_size);
            }
        }
        for (auto it=horizontal_segments.begin()+1;it!= horizontal_segments.end()-1;++it) {
            if (it->type == Segment::HANDLE) {
                it->spring().fixed(fixed_size);
            }
        }
        canvas.markDirty();
        return *this;
    }

    Grid& Grid::setExternalHandleFixedSize(int fixed_size) {
        if (vertical_segments.size()) {
            vertical_segments.front().spring().fixed(fixed_size);
            vertical_segments.back().spring().fixed(fixed_size);
        }
        if (horizontal_segments.size()) {
            horizontal_segments.front().spring().fixed(fixed_size);
            horizontal_segments.back().spring().fixed(fixed_size);
        }
        canvas.markDirty();
        return *this;
    }
    
    Segment& Grid::hseg(int index) {
        return horizontal_segments.at(index);
    }

    Segment& Grid::vseg(int index) {
        return vertical_segments.at(index);
    }

    bool Grid::contains(const Point &p) const {
//        std::cerr << "Grid::contains(...): p=" << p << " is in " << window << (window.contains(p) ? " yes" : " no") << std::endl;
//        std::cerr.flush();
        return window.contains(p);
    }

    WidgetIterator Grid::children() const {
        return WidgetIterator(new iterator(cell_map));
    }
    
    void Grid::layout() {
        
        auto spread = [](std::vector<Segment> &segments, Length length) {
            double target = length;

            double sum_absolute = 0.0f;
            double sum_relative = 0.0f;
            
            for (auto& segment: segments) {
                auto &spring = segment.spring();
                if (spring.type == Spring::FIXED_SIZE) {
                    sum_absolute += spring.fixed();
                }
                else {
                    sum_relative += spring.weight();
                }
            }

            // no error check for now assume it words
            
            double target_relative = target - sum_absolute;
            
            double conversion_factor = (target_relative <= 0) ? 0.0 : target_relative/sum_relative;
            
            Length pos { 0 };
            for (auto& segment: segments) {
                segment.p0(pos);
                auto &spring = segment.spring();
                if (spring.type == Spring::FIXED_SIZE) {
                    segment.size(spring.fixed());
                }
                else { // relative
                    segment.size(std::floor(conversion_factor * spring.weight()));
                }
                pos = pos + segment.size();
            }
        };
        
        spread(horizontal_segments, window.size().x());
        spread(vertical_segments,   window.size().y());
        
    }

    bool Grid::movableSplitters() const {
        return _movable_splitters;
    }
    
    Grid& Grid::movableSplitters(bool flag) {
        _movable_splitters = flag;
        return *this;
    }
    void Grid::applyGesture() {
        
        if (!gesture.resizing)
            return;
        
        //
        // there needs to be three segments:
        //
        //    cell1, handle, cell2
        //
        // cell1, cell2 are weight based
        //
        // we will adjust their weights so that
        // a proper growth is encoded
        //
        
        std::cout << "applying gesture to grid with size: " << horizontal_segments.size() << " x " << vertical_segments.size() << std::endl;
        
        auto index     = gesture.splitter.index;
        auto &segments = (gesture.splitter.horizontal() ? vertical_segments : horizontal_segments);
        auto delta     = (gesture.splitter.horizontal() ? (gesture.p1 - gesture.p0).y() : (gesture.p1 - gesture.p0).x()); ;
        
        
        auto &cell1  = segments.at(index-1);
        // auto &handle = segments.at(index);
        auto &cell2  = segments.at(index+1);
        
        //
        // current weight to pixel correspondence:
        //
        //
        
        // w1 + w2 ==
        if (cell1.spring().type != Spring::WEIGHT ||
            cell2.spring().type != Spring::WEIGHT) {
            std::cerr << "Warning: don't know how to apply gesture" << std::endl;
        }
            

        auto w1 = cell1.spring().weight();
        // auto w2 = cell2.spring.weight;
        auto l1 = (double) cell1.size();
        auto l2 = (double) cell2.size();
        
        auto ll1 = l1 + delta;
        auto ll2 = l2 - delta;
        
        if (ll1 < 0 || ll2 < 0) {
            std::cerr << "Warning: don't know how to apply gesture" << std::endl;
        }
        
        auto c = l1 / w1;
        
        auto ww1 = ll1 / c;
        auto ww2 = ll2 / c;
        
        cell1.spring().weight(ww1);
        cell2.spring().weight(ww2);

        //
        // w1 = c * l1
        //
        // ww1 = c
    }
    
    llsg::AxisAlignedBox Grid::handleRect(const Splitter& s) const {
        auto min = llsg::Vec2{ (double) window.min().x(), (double) window.min().y() };
        auto max = llsg::Vec2{ (double) window.max().x(), (double) window.max().y() };
        if (s.kind == Splitter::HORIZONTAL) {
            auto &segment = vertical_segments.at(s.index);
            auto y = segment.p0();
            auto h = segment.size();
            llsg::Vec2 p0 { min.x(), min.y() + y     };
            llsg::Vec2 p1 { max.x(), min.y() + y + h };
            return llsg::AxisAlignedBox { p0, p1 };
        }
        else  { // if (s.kind == Splitter::VERTICAL) {
            auto &segment = horizontal_segments.at(s.index);
            auto x = segment.p0();
            auto w = segment.size();
            llsg::Vec2 p0 { min.x() + x    , min.y()};
            llsg::Vec2 p1 { min.x() + x + w, max.y()};
            return llsg::AxisAlignedBox { p0, p1 };
        }
    }
    
    void Grid::prepareCanvas() {
    
        // for each pair:
        //
        // (comment for now) handle x handle
        // handle x cell
        // cell   x handle
        //
        
        std::vector<Splitter> splitters;
        for (auto &h: horizontal_segments) {
            if (h.type == Segment::HANDLE && h.spring().isFixed() && h.spring().fixed() > 0) {
                splitters.push_back(Splitter(h.index,Splitter::VERTICAL));
            }
        }
        for (auto &v: vertical_segments) {
            if (v.type == Segment::HANDLE && v.spring().isFixed() && v.spring().fixed() > 0) {
                splitters.push_back(Splitter(v.index,Splitter::HORIZONTAL));
            }
        }
        
        struct Info {
        public:
            Info() = default;
            Info(int index, const Splitter *splitter):
            index(index)
            {
                if (splitter)
                    splitters.push_back(*splitter);
            }
            Info(int index, std::vector<Splitter> &&splitters):
                index(index),
                splitters(std::move(splitters))
            {}
        public:
            int index { -1 };
            std::vector<Splitter> splitters;
        };

        
        auto data = std::vector<Info>{ Info(0,std::move(splitters)), Info(1,gesture.resizing ? &gesture.splitter : nullptr) };
        
        //
        // two groups: the first one with the current separation bars
        // the second one with the current moving bar
        //
        std::function<llsg::ElementIterator(llsg::Element*)> iter1 = [](llsg::Element* e) {
            return llsg::ElementIterator(e,1,1);
        };
        
        using document_type = d3cpp::Document<llsg::Element>;
        
        document_type document(&canvas.root);
        
        //
        auto groups_sel = document
        .selectAll(llsg::isGroup,iter1)
        .data(data);
        
        // exit
        groups_sel
        .exit()
        .remove([](llsg::Element* e) { e->remove(); });
        
        // enter
        groups_sel
        .enter()
        .append([](llsg::Element* parent, const Info& info) { return &parent->asGroup().g(); });
        

        std::function<std::vector<Info>(const Info&)> mapping = [](const Info &info) {
            std::vector<Info> forward_infos;
            if (info.splitters.size()) {
                for (auto &s: info.splitters) {
                    forward_infos.push_back(Info{info.index, &s});
                }
            }
            return forward_infos;
        };
        
        // update do nothing (we already have the groups)
        auto rectangle_sel = groups_sel
        .selectAll(llsg::isRect, iter1)
        .data(mapping);
        
        //
        auto &that = *this;

        // erase non matching splitters (no animation)
        rectangle_sel
        .exit()
        .remove([](llsg::Element *e) { e->remove(); });

        // append
        rectangle_sel
        .enter()
        .append([&](llsg::Element *parent, const Info &i) {
            auto &r = parent->asGroup().rect();
            r.data(any::Any(i.splitters.at(0))); // set splitter as the data
//            auto s = i.splitters.at(0);
//            auto bounds = that.handleRect(s);
//            r.pos(bounds.min());
//            r.size(bounds.size());
            return &r;
        });
        
        // update
        rectangle_sel
        .call([&](llsg::Element *e, const Info &i) {
            auto &r = e->asRectangle();
            auto s = i.splitters.at(0);
            auto bounds = that.handleRect(s);
            if (i.index == 0) {
                
                // regular
                r.pos(bounds.min());
                r.size(bounds.size());
                
                if (s == that.gesture.hover_splitter) {
                    r.style().color().reset(llsg::Color(1.0f));
                }
                else {
                    r.style().color().reset(llsg::Color(0.5f));
                }
            }
            else {
                // resizing splitter
                llsg::Vec2 delta = s.horizontal() ?
                llsg::Vec2 { 0, (that.gesture.p1 - that.gesture.p0).y() } :
                llsg::Vec2 { (that.gesture.p1 - that.gesture.p0).x(), 0 };
                
                bounds = bounds + delta;
                
                r.pos(bounds.min());
                r.size(bounds.size());
                r.style().color().reset(llsg::Color(0.2f));
            }
        });
        
        canvas.markDirty(false);
        
    }
    
    void Grid::onMousePress(const App &app) {
        // std::lock_guard<std::mutex> lock(mutex);
        
        if (!movableSplitters())
            return;
        
        auto mouse_pos = app.current_event_info.mouse_position;
        // auto modifiers = app.current_event_info.modifiers;
        
        llsg::GeometricTests g;
        auto e = g.firstHit(llsg::Vec2{(double)mouse_pos.x(), (double)mouse_pos.y()}, canvas.root);
        
        if (!e)
            return;
        
        // get object id
        if (!any::can_cast<Splitter>(e->data()))
            return;
        
        auto splitter = any::any_cast<Splitter>(e->data());
        
        std::cerr << "splitter: " << splitter.index << "  type: " << splitter.kind << std::endl;
        
        app.lock(this);
        
        gesture.resizing = true;
        gesture.splitter = splitter;
        gesture.p0       = llsg::Vec2{(double)mouse_pos.x(), (double)mouse_pos.y()};
        gesture.p1       = gesture.p0;
        
        canvas.markDirty();
    }
    
    void Grid::onMouseMove(const App &app) {
        auto mouse_pos = app.current_event_info.mouse_position;
        if (gesture.resizing) {
            std::cout << "Moving Splitter!" << std::endl;
            
            gesture.p1 = llsg::Vec2{(double)mouse_pos.x(), (double)mouse_pos.y()};
            auto delta = gesture.p1 - gesture.p0;
            if (gesture.splitter.horizontal()) {
                std::cerr << "Add dy: " << delta.y() << std::endl;
            }
            else if (gesture.splitter.vertical()) {
                std::cerr << "Add dx: " << delta.x() << std::endl;
            }
            canvas.markDirty();
        }
        else if (movableSplitters()) {
            llsg::GeometricTests g;
            auto e = g.firstHit(llsg::Vec2{(double)mouse_pos.x(), (double)mouse_pos.y()}, canvas.root);
            
            bool hovering_nothing = !e || !any::can_cast<Splitter>(e->data());
            
            if (!hovering_nothing) {
                gesture.hover_splitter = any::any_cast<Splitter>(e->data());
                canvas.markDirty();
            }
            else if (hovering_nothing && gesture.hover_splitter.kind != Splitter::NONE) {
                gesture.hover_splitter = Splitter{0,Splitter::NONE};
                canvas.markDirty();
            }
        }
    }

    void Grid::onMouseRelease(const App &app) {
        if (gesture.resizing) {
            app.lock(); // unlock
            applyGesture();
            gesture.resizing = false;
            
            // trigger resizing of children widgets
            this->sizeHint(window);
            std::cout << "Finished Resizing!" << std::endl;
        }
    }

    //--------------------------------------------------------------------------
    // Grid::iterator
    //--------------------------------------------------------------------------

    Grid::iterator::iterator(const container_type &container):
    current(container.cbegin()),
    end(container.cend())
    {}
    
    bool Grid::iterator::next(Widget* &widget) {
        if (current != end) {
            widget = current->second;
            ++current;
            return true;
        }
        else {
            return false;
        }
    }


}
