#include "canvas.hh"

namespace lluitk {

    //--------------------------------------------------------------------------
    // Canvas
    //--------------------------------------------------------------------------
    
    Canvas& Canvas::markDirty(bool flag) {
        dirty = flag;
        return *this;
    }
    
}