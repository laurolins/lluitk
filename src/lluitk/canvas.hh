#pragma once

#include "llsg/llsg.hh"

namespace lluitk {
    
    //---------------------------------------------------------
    // Canvas
    //---------------------------------------------------------
    
    struct Canvas {
        Canvas() = default;
        
        Canvas& markDirty(bool flag=true);
        
        bool          dirty   { true };
        llsg::Group   root;
    };
    
}