#include "lvgl.h"
#include "style.h"


static const lv_style_const_prop_t style_icon_btn_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(8),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
    LV_STYLE_CONST_RADIUS(LV_RADIUS_CIRCLE),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_icon_btn, style_icon_btn_props);


void style_init(void) {}