#pragma once

#include <map>

#include "simple_widget.hh"
#include "canvas.hh"

#include "llsg/llsg.hh"
#include "llsg/llsg_opengl.hh"

namespace lluitk {
    
    using GridLength = int;

    //---------------------------------------------------------
    // GridPoint
    //---------------------------------------------------------

    struct GridPoint {
        GridPoint() = default;
        GridPoint(GridLength x, GridLength y);
        
        GridLength x() const;
        GridLength y() const;

        GridPoint& x(const GridLength& x);
        GridPoint& y(const GridLength& y);
        
        bool operator<(const GridPoint& other) const;
        bool operator==(const GridPoint& other) const;

    public:
        GridLength _x { 0 };
        GridLength _y { 0 };
    };
    
    GridPoint operator+(const GridPoint& a, const GridPoint &b);
    
    using GridSize = GridPoint;
    
    //---------------------------------------------------------
    // Springs
    //---------------------------------------------------------
    
    struct Spring {
    public:
        enum Type { FIXED_SIZE, WEIGHT };
    public:
        Spring() = default;
        Spring(const Length& fixed_size, Type type);
        
        Spring& fixed(const Length& length);
        Spring& weight(const Length& w);
        
        Length weight() const;
        Length fixed() const;
        
        bool isFixed() const;
        bool isWeighted() const;
        
    public:
        Type        type { FIXED_SIZE };
        Length      length { 0.0f };
    };
    
    //---------------------------------------------------------
    // Segment
    //---------------------------------------------------------
    
    struct Segment {
    public:
        enum Type { HANDLE, CELL };
    public:
        Segment() = default;
        Segment(Type type, int index, const Spring& spring);
        
        Length p0() const;
        Length p1() const;
        Length size() const;

        Segment& p0(const Length &len);
        Segment& size(const Length& len);
        
        Spring&  spring();
        Segment& spring(const Spring& spring); // set spring

        const Spring& spring() const;
        
    public:
        Type         type  { HANDLE };
        int          index { -1 };
        Spring       _spring;
        Length       _p0;
        Length       _size;
    };
    

    
    //---------------------------------------------------------
    // Splitter
    //---------------------------------------------------------
    
    /*! \brief A splitter separates two cells
     */
    struct Splitter {
    public:
        enum Kind { HORIZONTAL, VERTICAL, NONE };
    public:
        Splitter() = default;
        Splitter(int index, Kind kind);
        bool operator==(const Splitter& other) const;
        
        
        bool valid() const;
        bool horizontal() const;
        bool vertical() const;
        
        
    public:
        Kind kind  { NONE };
        int  index { 0 }; // index of segment
    };

    
    //------------------------------------------------------------------------------
    // GridStyle
    //------------------------------------------------------------------------------
    
    struct GridStyle {
    public:
        GridStyle() = default;
    public:
        Color focused_splitter_color() const;
        Color live_splitter_color() const;
        Color phantom_splitter_color() const;
        GridStyle& focused_splitter_color(const Color& p);
        GridStyle& live_splitter_color(const Color& p);
        GridStyle& phantom_splitter_color(const Color& p);
    private:
        Color _focused_splitter_color { 0.6f };
        Color _live_splitter_color { 0.8f };
        Color _phantom_splitter_color { 0.4f };
    };
    
    //---------------------------------------------------------
    // Grid
    //---------------------------------------------------------
    
    
    struct Grid: public SimpleWidget {
    public:
        
        //
        struct iterator: public BaseWidgetIterator {
        public:
            using container_type = std::map<GridPoint, Widget*>;
            using base_iter_type = decltype(std::declval<container_type>().cbegin());
        public:
            iterator()=default;
            iterator(const container_type &container);
            bool next(Widget* &next_widget);
        public:
            base_iter_type current;
            base_iter_type end;
        };
    
    public:
        
        Grid() = default;
        Grid(const GridSize& size);
        
    public: // overload the children service
        
        Grid& setInternalHandleFixedSize(int fixed_size);
        Grid& setExternalHandleFixedSize(int fixed_size);

        Segment& hseg(int index);
        Segment& vseg(int index);

        bool contains(const Point& p) const;
        
        WidgetIterator children() const;

        void setCellWidget(const GridPoint& cell, Widget* widget);

        void swapWidget(const GridPoint& cell0, const GridPoint& cell1);

        void sizeHint(const Window &window);
        
        void render(); // assuming opengl context in pixel
                       // correct coordinates
        
        bool movableSplitters() const;
        Grid& movableSplitters(bool flag);
        
        void pre_render();
        
        GridStyle& grid_style();
        const GridStyle& grid_style() const;

    public:
        
        bool acceptsKeyEvents() const;
        void onKeyPress(const App &app);
        void onMousePress(const App &app);
        void onMouseMove(const App &app);
        void onMouseRelease(const App &app);

    private:
        void layout();
        void prepareCanvas();
        void applyGesture();
        
        llsg::AxisAlignedBox handleRect(const Splitter& splitter) const;
        

    public:

        Window window; // window of the grid...
        
        std::map<GridPoint, Widget*> cell_map;
    
        GridSize size { 0, 0 }; // rows and columns
        
        Canvas canvas;
        
        bool _movable_splitters { true };
        
        struct {
            bool         resizing { false };
            llsg::Vec2   p0;
            llsg::Vec2   p1;
            Splitter     splitter;
            Splitter     hover_splitter { 0, Splitter::NONE };
        } gesture ;

    private:
        std::vector<Segment> horizontal_segments;
        std::vector<Segment> vertical_segments;
        
        GridStyle _grid_style;
    };
    
}
