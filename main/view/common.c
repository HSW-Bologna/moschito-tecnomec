#include "view/view.h"
#include "common.h"


LV_IMG_DECLARE(img_erogators_inactive);
LV_IMG_DECLARE(img_erogators_1);
LV_IMG_DECLARE(img_erogators_2);
LV_IMG_DECLARE(img_erogation_1);
LV_IMG_DECLARE(img_erogation_2);
LV_IMG_DECLARE(img_dead_mosquito);
LV_IMG_DECLARE(img_live_mosquito);


void view_common_img_set_src(lv_obj_t *img, const lv_img_dsc_t *dsc) {
    if (lv_img_get_src(img) != dsc) {
        lv_img_set_src(img, dsc);
    }
}


void view_common_set_hidden(lv_obj_t *obj, uint8_t hidden) {
    if ((obj->flags & LV_OBJ_FLAG_HIDDEN) > 0 && !hidden) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if ((obj->flags & LV_OBJ_FLAG_HIDDEN) == 0 && hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}


lv_obj_t *view_common_menu_button(lv_obj_t *root, const lv_img_dsc_t *img_dsc, int id) {
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 90, 90);
    lv_obj_t *img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, img_dsc);
    view_register_object_default_callback(btn, id);
    return btn;
}


void view_common_erogator_graphic_create(lv_obj_t *root, view_common_erogator_graphic_t *pointers) {
    lv_obj_t *img = lv_img_create(root);
    lv_img_set_src(img, &img_erogators_inactive);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 14);
    pointers->img_erogators = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_erogation_1);
    lv_obj_align_to(img, pointers->img_erogators, LV_ALIGN_TOP_LEFT, -50, -15);
    pointers->img_erogation_1 = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_erogation_2);
    lv_obj_align_to(img, pointers->img_erogators, LV_ALIGN_TOP_RIGHT, 65, -30);
    pointers->img_erogation_2 = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_dead_mosquito);
    lv_obj_align_to(img, pointers->img_erogation_1, LV_ALIGN_TOP_LEFT, -30, -30);
    pointers->img_dead_mosquito = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_live_mosquito);
    lv_obj_align_to(img, pointers->img_erogation_2, LV_ALIGN_TOP_RIGHT, 15, -15);
    pointers->img_live_mosquito = img;
}


void view_common_update_erogator_graphic(view_common_erogator_graphic_t *pointers, erogators_state_t state) {
    switch (state) {
        case EROGATORS_STATE_OFF:
            view_common_img_set_src(pointers->img_erogators, &img_erogators_inactive);
            view_common_set_hidden(pointers->img_erogation_1, 1);
            view_common_set_hidden(pointers->img_erogation_2, 1);
            view_common_set_hidden(pointers->img_dead_mosquito, 1);
            view_common_set_hidden(pointers->img_live_mosquito, 1);
            break;

        case EROGATORS_STATE_1:
            view_common_img_set_src(pointers->img_erogators, &img_erogators_1);
            view_common_set_hidden(pointers->img_erogation_1, 0);
            view_common_set_hidden(pointers->img_erogation_2, 1);
            view_common_set_hidden(pointers->img_dead_mosquito, 0);
            view_common_set_hidden(pointers->img_live_mosquito, 1);
            break;

        case EROGATORS_STATE_2:
            view_common_img_set_src(pointers->img_erogators, &img_erogators_2);
            view_common_set_hidden(pointers->img_erogation_1, 1);
            view_common_set_hidden(pointers->img_erogation_2, 0);
            view_common_set_hidden(pointers->img_dead_mosquito, 1);
            view_common_set_hidden(pointers->img_live_mosquito, 0);
            break;
    }
}