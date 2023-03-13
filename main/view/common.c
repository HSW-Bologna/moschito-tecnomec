#include "view/view.h"
#include "common.h"
#include "theme/style.h"
#include "view/intl/intl.h"


LV_IMG_DECLARE(img_erogators_inactive);
LV_IMG_DECLARE(img_erogators_1);
LV_IMG_DECLARE(img_erogators_2);
LV_IMG_DECLARE(img_erogation_1);
LV_IMG_DECLARE(img_erogation_2);
LV_IMG_DECLARE(img_erogators_1_warning);
LV_IMG_DECLARE(img_erogators_2_warning);
LV_IMG_DECLARE(img_dead_mosquito);
LV_IMG_DECLARE(img_live_mosquito);
LV_IMG_DECLARE(img_locked_pump);
LV_IMG_DECLARE(img_warning);
LV_IMG_DECLARE(img_signal_off_sm);
LV_IMG_DECLARE(img_signal_off_off);


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


void view_common_set_checked(lv_obj_t *obj, uint8_t checked) {
    if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) > 0 && !checked) {
        lv_obj_clear_state(obj, LV_STATE_CHECKED);
    } else if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) == 0 && checked) {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
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


lv_obj_t *view_common_option_button(lv_obj_t *root, int id) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_add_style(btn, (lv_style_t *)&style_option_btn, LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, id);
    return btn;
}


lv_obj_t *view_common_modify_button(lv_obj_t *root, const char *text, const lv_font_t *font, int id) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 90, 90);
    lv_obj_add_style(btn, (lv_style_t *)&style_modify_btn, LV_STATE_DEFAULT);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lbl, STYLE_BG_COLOR, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);
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
    pointers->img_erogation_1 = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_erogation_2);
    pointers->img_erogation_2 = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_dead_mosquito);
    pointers->img_dead_mosquito = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_live_mosquito);
    pointers->img_live_mosquito = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_locked_pump);
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    pointers->img_locked_pump = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_warning);
    pointers->img_pump_warning = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_signal_off_off);
    pointers->img_pump_off = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_signal_off_sm);
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    pointers->img_erogator_1_off = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_erogators_1_warning);
    pointers->img_erogator_1_warning = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_warning);
    pointers->img_erogator_1_warning_icon = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_signal_off_sm);
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    pointers->img_erogator_2_off = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_erogators_2_warning);
    pointers->img_erogator_2_warning = img;

    img = lv_img_create(root);
    lv_img_set_src(img, &img_warning);
    pointers->img_erogator_2_warning_icon = img;

    view_common_erogator_graphic_realign(pointers);
}


void view_common_erogator_graphic_realign(view_common_erogator_graphic_t *pointers) {
    lv_obj_align(pointers->img_locked_pump, LV_ALIGN_CENTER, 0, -48);
    lv_obj_align(pointers->img_pump_warning, LV_ALIGN_CENTER, -100, 90);
    lv_obj_align(pointers->img_pump_off, LV_ALIGN_CENTER, 50, 100);

    lv_obj_align_to(pointers->img_erogator_1_warning_icon, pointers->img_erogators, LV_ALIGN_TOP_LEFT, -80, -30);
    lv_obj_align_to(pointers->img_erogator_1_off, pointers->img_erogator_1_warning_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_align_to(pointers->img_erogator_2_warning_icon, pointers->img_erogators, LV_ALIGN_TOP_RIGHT, 80, -30);
    lv_obj_align_to(pointers->img_erogator_2_off, pointers->img_erogator_2_warning_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    lv_obj_align_to(pointers->img_erogation_1, pointers->img_erogators, LV_ALIGN_TOP_LEFT, -50, -15);
    lv_obj_align_to(pointers->img_erogation_2, pointers->img_erogators, LV_ALIGN_TOP_RIGHT, 65, -30);
    lv_obj_align_to(pointers->img_erogator_1_warning, pointers->img_erogators, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align_to(pointers->img_erogator_2_warning, pointers->img_erogators, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_align_to(pointers->img_dead_mosquito, pointers->img_erogation_1, LV_ALIGN_TOP_LEFT, -30, -30);
    lv_obj_align_to(pointers->img_live_mosquito, pointers->img_erogation_2, LV_ALIGN_TOP_RIGHT, 15, -15);
}


void view_common_update_erogator_graphic(view_common_erogator_graphic_t *pointers, erogators_state_t state,
                                         uint8_t missing_water_alarm, uint8_t missing_product_1,
                                         uint8_t missing_product_2) {
    if (missing_water_alarm) {
        view_common_set_hidden(pointers->img_locked_pump, 0);
        view_common_set_hidden(pointers->img_pump_warning, 0);
        view_common_set_hidden(pointers->img_pump_off, 0);

        view_common_set_hidden(pointers->img_erogators, 1);
        view_common_set_hidden(pointers->img_erogation_1, 1);
        view_common_set_hidden(pointers->img_erogation_2, 1);
        view_common_set_hidden(pointers->img_dead_mosquito, 1);
        view_common_set_hidden(pointers->img_live_mosquito, 1);
        view_common_set_hidden(pointers->img_erogator_1_off, 1);
        view_common_set_hidden(pointers->img_erogator_1_warning, 1);
        view_common_set_hidden(pointers->img_erogator_1_warning_icon, 1);
        view_common_set_hidden(pointers->img_erogator_2_off, 1);
        view_common_set_hidden(pointers->img_erogator_2_warning, 1);
        view_common_set_hidden(pointers->img_erogator_2_warning_icon, 1);
    } else {
        view_common_set_hidden(pointers->img_locked_pump, 1);
        view_common_set_hidden(pointers->img_pump_warning, 1);
        view_common_set_hidden(pointers->img_pump_off, 1);

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


        if (missing_product_1) {
            view_common_set_hidden(pointers->img_erogator_1_off, 0);
            view_common_set_hidden(pointers->img_erogator_1_warning, 0);
            view_common_set_hidden(pointers->img_erogator_1_warning_icon, 0);
        } else {
            view_common_set_hidden(pointers->img_erogator_1_off, 1);
            view_common_set_hidden(pointers->img_erogator_1_warning, 1);
            view_common_set_hidden(pointers->img_erogator_1_warning_icon, 1);
        }

        if (missing_product_2) {
            view_common_set_hidden(pointers->img_erogator_2_off, 0);
            view_common_set_hidden(pointers->img_erogator_2_warning, 0);
            view_common_set_hidden(pointers->img_erogator_2_warning_icon, 0);
        } else {
            view_common_set_hidden(pointers->img_erogator_2_off, 1);
            view_common_set_hidden(pointers->img_erogator_2_warning, 1);
            view_common_set_hidden(pointers->img_erogator_2_warning_icon, 1);
        }
    }
}


lv_obj_t *view_common_horizontal_line(lv_obj_t *root) {
    static lv_point_t points[2] = {{0, 0}, {270, 0}};
    lv_obj_t         *line      = lv_line_create(root);
    lv_line_set_points(line, points, 2);
    lv_obj_set_style_line_color(line, STYLE_FG_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(line, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(line, 1, LV_STATE_DEFAULT);
    return line;
}


lv_obj_t *view_common_vertical_parameter_widget(lv_obj_t *root, lv_obj_t **lbl, int id_minus, int id_plus) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_size(cont, 80, 220);

    *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(*lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_obj_center(*lbl);

    lv_obj_t *btn = view_common_modify_button(cont, LV_SYMBOL_MINUS, STYLE_FONT_HUGE, id_minus);
    lv_obj_set_size(btn, 72, 72);
    lv_obj_align_to(btn, *lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    btn = view_common_modify_button(cont, LV_SYMBOL_PLUS, STYLE_FONT_HUGE, id_plus);
    lv_obj_set_size(btn, 72, 72);
    lv_obj_align_to(btn, *lbl, LV_ALIGN_OUT_TOP_MID, 0, -8);

    return cont;
}


lv_obj_t *view_common_horizontal_parameter_widget(lv_obj_t *root, lv_obj_t **lbl, int id_minus, int id_plus) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_size(cont, 264, 80);

    *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(*lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_obj_center(*lbl);

    lv_obj_t *btn = view_common_modify_button(cont, LV_SYMBOL_MINUS, STYLE_FONT_HUGE, id_minus);
    lv_obj_set_size(btn, 72, 72);
    lv_obj_align_to(btn, *lbl, LV_ALIGN_OUT_LEFT_MID, -8, 0);

    btn = view_common_modify_button(cont, LV_SYMBOL_PLUS, STYLE_FONT_HUGE, id_plus);
    lv_obj_set_size(btn, 72, 72);
    lv_obj_align_to(btn, *lbl, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    return cont;
}


lv_obj_t *view_common_program_label(model_t *pmodel, lv_obj_t *root, lv_color_t color, unsigned int num) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_style_radius(cont, 4, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 100, 20);
    lv_obj_set_style_bg_color(cont, color, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_TINY, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lbl, "%s %i", view_intl_get_string(pmodel, STRINGS_PROGRAMMA), num + 1);
    lv_obj_center(lbl);

    lv_obj_align(cont, LV_ALIGN_TOP_MID, 24, 28);

    return cont;
}