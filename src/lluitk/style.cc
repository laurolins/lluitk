#include "style.hh"

namespace  lluitk {
 
    //------------------------------------------------------------------------------
    // Style
    //------------------------------------------------------------------------------
    
    WidgetStyle WidgetStyle::defaultStyle() {
        WidgetStyle style;
        style.fgcolor().reset(Color(0.0f));
        style.bgcolor().reset(Color(0.8f));
        style.fontMode().reset(FontMode());
        style.fontSize().reset(FontSize());
        style.typeface().reset(Typeface());
        return style;
    }
    
    const ColorP& WidgetStyle::fgcolor() const  {
        return _fgcolor;
    }
    
    const ColorP& WidgetStyle::bgcolor() const  {
        return _bgcolor;
    }
    
    const TypefaceP& WidgetStyle::typeface() const {
        return _typeface;
    }
    
    const FontSizeP& WidgetStyle::fontSize() const {
        return _font_size;
    }
    
    const FontModeP& WidgetStyle::fontMode() const {
        return _font_mode;
    }
    
    ColorP& WidgetStyle::fgcolor() {
        return _fgcolor;
    }
    
    ColorP& WidgetStyle::bgcolor() {
        return _bgcolor;
    }
    
    TypefaceP& WidgetStyle::typeface() {
        return _typeface;
    }
    
    FontSizeP& WidgetStyle::fontSize() {
        return _font_size;
    }
    
    FontModeP& WidgetStyle::fontMode() {
        return _font_mode;
    }
    
    WidgetStyle WidgetStyle::operator+(const WidgetStyle& other) const {
        WidgetStyle result(*this);
        result._fgcolor   += other._fgcolor;
        result._bgcolor   += other._bgcolor;
        result._typeface  += other._typeface;
        result._font_size += other._font_size;
        result._font_mode += other._font_mode;
        return result;
    }

}