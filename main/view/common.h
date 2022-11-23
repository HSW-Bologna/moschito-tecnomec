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
} view_common_erogator_graphic_t;


void      view_common_img_set_src(lv_obj_t *img, const lv_img_dsc_t *dsc);
void      view_common_set_hidden(lv_obj_t *obj, uint8_t hidden);
lv_obj_t *view_common_menu_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, int id);
void      view_common_erogator_graphic_create(lv_obj_t *root, view_common_erogator_graphic_t *pointers);
void      view_common_update_erogator_graphic(view_common_erogator_graphic_t *pointers, erogators_state_t state);


#endif