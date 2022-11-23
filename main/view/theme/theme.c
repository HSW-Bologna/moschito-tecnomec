#include "theme.h"
#include "style.h"


static const lv_style_const_prop_t style_obj_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_BG_COLOR),
    LV_STYLE_CONST_TEXT_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_RADIUS(0),
    LV_STYLE_CONST_NULL,
};
static LV_STYLE_CONST_INIT(style_obj, style_obj_props);

static const lv_style_const_prop_t style_btn_props[] = {
    LV_STYLE_CONST_BG_COLOR(LV_COLOR_MAKE(0, 0, 0)),
    LV_STYLE_CONST_PAD_BOTTOM(8),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
    LV_STYLE_CONST_RADIUS(16),
    LV_STYLE_CONST_BORDER_WIDTH(4),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_NULL,
};
static LV_STYLE_CONST_INIT(style_btn, style_btn_props);

static const lv_style_const_prop_t style_pressed_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_FADED_COLOR),
    LV_STYLE_CONST_NULL,
};
static LV_STYLE_CONST_INIT(style_btn_pressed, style_pressed_props);

static const lv_style_const_prop_t style_checked_props[] = {
    LV_STYLE_CONST_BG_COLOR(STYLE_CHECKED_COLOR),
    LV_STYLE_CONST_BORDER_WIDTH(2),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_FADED_COLOR),
    LV_STYLE_CONST_NULL,
};
static LV_STYLE_CONST_INIT(style_btn_checked, style_checked_props);

static const lv_style_const_prop_t style_disabled_props[] = {
    LV_STYLE_CONST_OPA(LV_OPA_60),
    LV_STYLE_CONST_NULL,
};
static LV_STYLE_CONST_INIT(style_disabled, style_disabled_props);


static void theme_apply_cb(lv_theme_t *th, lv_obj_t *obj);


void theme_init(lv_disp_t *disp) {
    lv_theme_t *th = lv_theme_basic_init(disp);
    // lv_theme_t *th = lv_theme_default_init(disp, lv_color_make(0x5e, 0x37, 0xd8), lv_color_make(20, 20, 60), 1,
    // &lv_font_montserrat_32);

    /*Initialize the new theme from the current theme*/
    static lv_theme_t th_new;
    th_new = *th;

    /*Set the parent theme and the style apply callback for the new theme*/
    lv_theme_set_parent(&th_new, th);
    lv_theme_set_apply_cb(&th_new, theme_apply_cb);

    /*Assign the new theme the the current display*/
    lv_disp_set_theme(disp, &th_new);
}


static void theme_apply_cb(lv_theme_t *th, lv_obj_t *obj) {
    lv_obj_add_style(obj, (lv_style_t *)&style_obj, LV_STATE_DEFAULT);

    if (lv_obj_check_type(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, (lv_style_t *)&style_btn, LV_STATE_DEFAULT);
        lv_obj_add_style(obj, (lv_style_t *)&style_btn_pressed, LV_STATE_PRESSED);
        lv_obj_add_style(obj, (lv_style_t *)&style_disabled, LV_STATE_DISABLED);
        lv_obj_add_style(obj, (lv_style_t *)&style_btn_checked, LV_STATE_CHECKED);
    }
}