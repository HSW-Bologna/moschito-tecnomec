#ifndef STYLE_H_INCLUDED
#define STYLE_H_INCLUDED

#include "lvgl.h"


#define STYLE_BG_COLOR      ((lv_color_t)LV_COLOR_MAKE(0x00, 0x00, 0x00))
#define STYLE_FG_COLOR      ((lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0xFF))
#define STYLE_FADED_COLOR   ((lv_color_t)LV_COLOR_MAKE(0x50, 0x50, 0x50))
#define STYLE_CHECKED_COLOR ((lv_color_t)LV_COLOR_MAKE(0x30, 0x30, 0x30))

#define LV_STYLE_CONST_NULL                                                                                            \
    {                                                                                                                  \
        LV_STYLE_PROP_INV, {                                                                                           \
            0                                                                                                          \
        }                                                                                                              \
    }


extern const lv_style_t style_icon_btn;


void style_init(void);

#endif