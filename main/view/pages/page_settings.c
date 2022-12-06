#include <time.h>
#include <stdlib.h>
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "view/common.h"
#include "view/view_types.h"
#include "view/theme/style.h"
#include "view/intl/intl.h"
#include "gel/pagemanager/page_manager.h"


LV_IMG_DECLARE(img_home_icon);
LV_IMG_DECLARE(img_italiano_sm);
LV_IMG_DECLARE(img_english_sm);
LV_IMG_DECLARE(img_check_sm);
LV_IMG_DECLARE(img_rondine);
LV_IMG_DECLARE(img_brightness_sm);
LV_IMG_DECLARE(img_date);
LV_IMG_DECLARE(img_time);
LV_IMG_DECLARE(img_erogators_sm);


enum {
    HOME_BTN_ID,
    TIME_TIMER_ID,
    LANGUAGE_BTN_ID,
    BACKLIGHT_BTN_ID,
    DATE_BTN_ID,
    TIME_BTN_ID,
    EROGATORS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_timer_t *timer_time;
};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static void      update_time(model_t *pmodel, struct page_data *pdata);
static void      label_modify_preview(lv_obj_t *root, int32_t delta_y, const char *desc, int value);
static lv_obj_t *product_percentage(lv_obj_t *root, model_t *pmodel, uint16_t percentage);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *img, *checkbox, *line, *obj;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 64, 64);
    img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, &img_home_icon);
    lv_obj_add_style(btn, (lv_style_t *)&style_icon_btn, LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, HOME_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -4, 4);


    btn = view_common_option_button(lv_scr_act(), LANGUAGE_BTN_ID);
    lv_obj_set_size(btn, 96, 60);

    img = lv_img_create(btn);
    lv_img_set_src(img, &img_italiano_sm);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    checkbox = lv_checkbox_create(btn);
    lv_checkbox_set_text(checkbox, "");
    lv_obj_set_style_bg_img_src(checkbox, &img_check_sm, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(checkbox, 16, 16);
    lv_obj_align_to(checkbox, img, LV_ALIGN_OUT_RIGHT_MID, 24, 0);
    view_common_set_checked(checkbox, model_get_language(pmodel) == LANGUAGE_ITALIANO);

    img = lv_img_create(btn);
    lv_img_set_src(img, &img_english_sm);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);

    checkbox = lv_checkbox_create(btn);
    lv_checkbox_set_text(checkbox, "");
    lv_obj_set_style_bg_img_src(checkbox, &img_check_sm, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(checkbox, 16, 16);
    lv_obj_align_to(checkbox, img, LV_ALIGN_OUT_RIGHT_MID, 24, 0);
    view_common_set_checked(checkbox, model_get_language(pmodel) == LANGUAGE_ENGLISH);

    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 8, 32);


    line = view_common_horizontal_line(lv_scr_act());
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 100);


    btn = view_common_option_button(lv_scr_act(), BACKLIGHT_BTN_ID);

    lv_obj_set_size(btn, 274, 32);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    obj = lv_obj_create(btn);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_border_width(obj, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, STYLE_FG_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 80, 24);

    lbl = lv_label_create(obj);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_DISPLAY));
    lv_obj_center(lbl);

    lv_obj_align(obj, LV_ALIGN_LEFT_MID, 0, 0);

    img = lv_img_create(btn);
    lv_img_set_src(img, &img_brightness_sm);
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, -48, 0);

    obj = view_common_modify_button(btn, LV_SYMBOL_MINUS, STYLE_FONT_SMALL, -1);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align_to(obj, img, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    obj = view_common_modify_button(btn, LV_SYMBOL_PLUS, STYLE_FONT_SMALL, -1);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align_to(obj, img, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    line = view_common_horizontal_line(lv_scr_act());
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    time_t    time_now = time(NULL);
    struct tm tm_info  = *localtime(&time_now);


    btn = view_common_option_button(lv_scr_act(), DATE_BTN_ID);

    img = lv_img_create(btn);
    lv_img_set_src(img, &img_date);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

    label_modify_preview(btn, -26, "AA", tm_info.tm_year + 1900);
    label_modify_preview(btn, 0, "MM", tm_info.tm_mon + 1);
    label_modify_preview(btn, 26, "GG", tm_info.tm_mday);

    lv_obj_set_size(btn, 274, 80);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    line = view_common_horizontal_line(lv_scr_act());
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    btn = view_common_option_button(lv_scr_act(), TIME_BTN_ID);

    img = lv_img_create(btn);
    lv_img_set_src(img, &img_time);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

    label_modify_preview(btn, -26, "HH", tm_info.tm_hour);
    label_modify_preview(btn, 0, "mm", tm_info.tm_min);
    label_modify_preview(btn, 26, "SEC", tm_info.tm_sec);

    lv_obj_set_size(btn, 274, 80);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    line = view_common_horizontal_line(lv_scr_act());
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    btn = view_common_option_button(lv_scr_act(), EROGATORS_BTN_ID);

    lv_obj_set_size(btn, 274, 100);
    img = lv_img_create(btn);
    lv_img_set_src(img, &img_erogators_sm);
    lv_obj_center(img);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_obj_t *perc_1 = product_percentage(btn, pmodel, model_get_erogator_percentage(pmodel, EROGATOR_1));
    lv_obj_align(perc_1, LV_ALIGN_BOTTOM_LEFT, 0, -4);

    lv_obj_t *perc_2 = product_percentage(btn, pmodel, model_get_erogator_percentage(pmodel, EROGATOR_2));
    lv_obj_align(perc_2, LV_ALIGN_BOTTOM_RIGHT, 0, -4);


    line = view_common_horizontal_line(lv_scr_act());
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_rondine);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 32, -4);


    update_page(pmodel, pdata);
    update_time(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER: {
            update_time(pmodel, pdata);
            break;
        }

        case VIEW_EVENT_CODE_UPDATE:
            update_page(pmodel, pdata);
            break;

        case VIEW_EVENT_CODE_LVGL: {
            switch (event.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case HOME_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case LANGUAGE_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_language;
                            break;

                        case BACKLIGHT_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_display;
                            break;

                        case DATE_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_date;
                            break;

                        case TIME_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_time;
                            break;

                        case EROGATORS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_erogators_percentage;
                            break;

                        default:
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return msg;
}


static void update_page(model_t *pmodel, struct page_data *pdata) {}


static void update_time(model_t *pmodel, struct page_data *pdata) {
    time_t    time_now = time(NULL);
    struct tm tm_info  = *localtime(&time_now);

    lv_label_set_text_fmt(pdata->lbl_time, "%i.%i.%i - %02i:%02i:%02i", tm_info.tm_year + 1900, tm_info.tm_mon + 1,
                          tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
}


static void label_modify_preview(lv_obj_t *root, int32_t delta_y, const char *desc, int value) {
    lv_obj_t *lbl = lv_label_create(root);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, 32);
    lv_label_set_text_fmt(lbl, "%02i", value);
    lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, -44, delta_y);

    lbl = lv_label_create(root);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_RIGHT, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, 32);
    lv_label_set_text(lbl, desc);
    lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, -124, delta_y);

    lv_obj_t *obj = view_common_modify_button(root, LV_SYMBOL_MINUS, STYLE_FONT_SMALL, -1);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -92, delta_y);

    obj = view_common_modify_button(root, LV_SYMBOL_PLUS, STYLE_FONT_SMALL, -1);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -4, delta_y);
}


static lv_obj_t *product_percentage(lv_obj_t *root, model_t *pmodel, uint16_t percentage) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_size(cont, 100, 38);
    lv_obj_set_style_pad_all(cont, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lbl, "%3i", percentage);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -2);

    lv_obj_t *btn = view_common_modify_button(cont, LV_SYMBOL_MINUS, STYLE_FONT_SMALL, -1);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(btn, 20, 20);
    lv_obj_align_to(btn, lbl, LV_ALIGN_OUT_LEFT_MID, -4, 0);

    btn = view_common_modify_button(cont, LV_SYMBOL_PLUS, STYLE_FONT_SMALL, -1);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(btn, 20, 20);
    lv_obj_align_to(btn, lbl, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    lv_obj_t *lbl_top = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl_top, STYLE_FONT_TINY, LV_STATE_DEFAULT);
    lv_obj_align(lbl_top, LV_ALIGN_TOP_MID, 0, 2);
    lv_label_set_text(lbl_top, view_intl_get_string(pmodel, STRINGS_PERC_PRODOTTO));

    return cont;
}


static void close_page(void *args) {
    struct page_data *pdata = args;
    lv_timer_pause(pdata->timer_time);
    lv_obj_clean(lv_scr_act());
}


static void destroy_page(void *args, void *extra) {
    struct page_data *pdata = args;
    lv_timer_del(pdata->timer_time);
    free(pdata);
    free(extra);
}


const pman_page_t page_settings = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};