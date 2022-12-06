#ifndef STYLE_H_INCLUDED
#define STYLE_H_INCLUDED

#include "lvgl.h"


#define STYLE_BG_COLOR         ((lv_color_t)LV_COLOR_MAKE(0x00, 0x00, 0x00))
#define STYLE_FG_COLOR         ((lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0xFF))
#define STYLE_FADED_COLOR      ((lv_color_t)LV_COLOR_MAKE(0x50, 0x50, 0x50))
#define STYLE_CHECKED_COLOR    ((lv_color_t)LV_COLOR_MAKE(0x30, 0x30, 0x30))
#define STYLE_EROGATOR_1_COLOR ((lv_color_t)LV_COLOR_MAKE(0xE1, 0x0B, 0x17))
#define STYLE_EROGATOR_2_COLOR ((lv_color_t)LV_COLOR_MAKE(0x00, 0x95, 0x40))

#define STYLE_FONT_TINY   &lv_font_montserrat_10
#define STYLE_FONT_SMALL  &lv_font_montserrat_16
#define STYLE_FONT_MEDIUM &lv_font_montserrat_24
#define STYLE_FONT_BIG    &lv_font_montserrat_32
#define STYLE_FONT_HUGE   &lv_font_montserrat_48

#define LV_STYLE_CONST_NULL                                                                                            \
    {                                                                                                                  \
        LV_STYLE_PROP_INV, {                                                                                           \
            0                                                                                                          \
        }                                                                                                              \
    }


extern const lv_style_t style_icon_btn;
extern const lv_style_t style_option_btn;
extern const lv_style_t style_modify_btn;
extern const lv_style_t style_border;
extern const lv_style_t style_bordered_container;
extern const lv_style_t style_alpha_icon;


void style_init(void);

#endif