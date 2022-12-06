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


static const lv_style_const_prop_t style_option_btn_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(4), LV_STYLE_CONST_PAD_TOP(4), LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),  LV_STYLE_CONST_RADIUS(4),  LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_option_btn, style_option_btn_props);


static const lv_style_const_prop_t style_modify_btn_props[] = {
    LV_STYLE_CONST_RADIUS(4),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_BG_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_TEXT_COLOR(STYLE_BG_COLOR),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_modify_btn, style_modify_btn_props);


static const lv_style_const_prop_t style_border_props[] = {
    LV_STYLE_CONST_RADIUS(16),
    LV_STYLE_CONST_BORDER_WIDTH(4),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_border, style_border_props);


static const lv_style_const_prop_t style_alpha_icon_props[] = {
    LV_STYLE_CONST_IMG_RECOLOR_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_IMG_RECOLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_alpha_icon, style_alpha_icon_props);


static const lv_style_const_prop_t style_bordered_container_props[] = {
    LV_STYLE_CONST_RADIUS(4),
    LV_STYLE_CONST_BORDER_WIDTH(2),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_NULL,
};
LV_STYLE_CONST_INIT(style_bordered_container, style_bordered_container_props);

void style_init(void) {}