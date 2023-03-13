#ifndef VIEW_COMMON_H_INCLUDED
#define VIEW_COMMON_H_INCLUDED


#include "lvgl.h"
#include "model/model.h"


typedef struct {
    lv_obj_t *img_erogators;
    lv_obj_t *img_erogator_1;
    lv_obj_t *img_erogator_2;
    lv_obj_t *img_erogation_1;
    lv_obj_t *img_erogation_2;
    lv_obj_t *img_dead_mosquito;
    lv_obj_t *img_live_mosquito;
    lv_obj_t *img_locked_pump;
    lv_obj_t *img_pump_warning;
    lv_obj_t *img_pump_off;
    lv_obj_t *img_erogator_1_warning;
    lv_obj_t *img_erogator_1_warning_icon;
    lv_obj_t *img_erogator_1_off;
    lv_obj_t *img_erogator_2_warning;
    lv_obj_t *img_erogator_2_warning_icon;
    lv_obj_t *img_erogator_2_off;
} view_common_erogator_graphic_t;


typedef struct {
    size_t     num;
    erogator_t erogator;
} view_common_program_t;


void      view_common_img_set_src(lv_obj_t *img, const lv_img_dsc_t *dsc);
void      view_common_set_hidden(lv_obj_t *obj, uint8_t hidden);
lv_obj_t *view_common_menu_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, int id);
void      view_common_erogator_graphic_create(lv_obj_t *root, view_common_erogator_graphic_t *pointers);
void      view_common_erogator_graphic_realign(view_common_erogator_graphic_t *pointers);
void      view_common_update_erogator_graphic(view_common_erogator_graphic_t *pointers, erogators_state_t state,
                                              uint8_t missing_water_alarm, uint8_t missing_product_1, uint8_t missing_product_2);
lv_obj_t *view_common_option_button(lv_obj_t *root, int id);
lv_obj_t *view_common_modify_button(lv_obj_t *root, const char *text, const lv_font_t *font, int id);
void      view_common_set_checked(lv_obj_t *obj, uint8_t checked);
lv_obj_t *view_common_horizontal_line(lv_obj_t *root);
lv_obj_t *view_common_vertical_parameter_widget(lv_obj_t *root, lv_obj_t **lbl, int id_minus, int id_plus);
lv_obj_t *view_common_horizontal_parameter_widget(lv_obj_t *root, lv_obj_t **lbl, int id_minus, int id_plus);
lv_obj_t *view_common_program_label(model_t *pmodel, lv_obj_t *root, lv_color_t color, unsigned int num);


#endif