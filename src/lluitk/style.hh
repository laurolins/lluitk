#pragma once 

#include "base.hh"

namespace lluitk {
    
    //------------------------------------------------------------------------------
    // Style
    //------------------------------------------------------------------------------
    
    struct WidgetStyle {
    public:
        
        static WidgetStyle defaultStyle();
        
    public:
        WidgetStyle() = default;
        
        //        Style(const Style& other);
        //        Style& operator=(const Style& other);
        
        const ColorP&    fgcolor() const;
        const ColorP&    bgcolor() const;
        const TypefaceP& typeface() const;
        const FontSizeP& fontSize() const;
        const FontModeP& fontMode() const;
        
        ColorP&    fgcolor();
        ColorP&    bgcolor();
        TypefaceP& typeface();
        FontSizeP& fontSize();
        FontModeP& fontMode();
        
        WidgetStyle operator+(const WidgetStyle& other) const;
        
    public:
        ColorP    _fgcolor;
        ColorP    _bgcolor;
        TypefaceP _typeface;
        FontSizeP _font_size;
        FontModeP _font_mode;
    };
    
}