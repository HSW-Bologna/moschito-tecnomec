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


LV_IMG_DECLARE(img_italiano_md);
LV_IMG_DECLARE(img_english_md);
LV_IMG_DECLARE(img_check_md);
LV_IMG_DECLARE(img_return);
LV_IMG_DECLARE(img_confirm);


enum {
    CB_BTN_ID,
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *cb_italiano;
    lv_obj_t *cb_english;

    lv_timer_t *timer_time;

    uint16_t language;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->language = model_get_language(pmodel);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *img, *checkbox;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 32, 32);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_LINGUA));

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_english_md);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 24, 72);
    lv_obj_t *prev_img = img;

    checkbox = lv_checkbox_create(lv_scr_act());
    lv_checkbox_set_text(checkbox, "");
    lv_obj_set_style_bg_img_src(checkbox, &img_check_md, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_size(checkbox, 40, 40);
    lv_obj_set_style_pad_all(checkbox, 8, LV_STATE_DEFAULT | LV_PART_INDICATOR);
    lv_obj_align_to(checkbox, img, LV_ALIGN_OUT_RIGHT_MID, 64, 0);
    view_register_object_default_callback_with_number(checkbox, CB_BTN_ID, LANGUAGE_ENGLISH);
    pdata->cb_english = checkbox;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_italiano_md);
    lv_obj_align_to(img, prev_img, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    checkbox = lv_checkbox_create(lv_scr_act());
    lv_checkbox_set_text(checkbox, "");
    lv_obj_set_style_bg_img_src(checkbox, &img_check_md, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_size(checkbox, 40, 40);
    lv_obj_set_style_pad_all(checkbox, 8, LV_STATE_DEFAULT | LV_PART_INDICATOR);
    lv_obj_align_to(checkbox, img, LV_ALIGN_OUT_RIGHT_MID, 64, 0);
    view_register_object_default_callback_with_number(checkbox, CB_BTN_ID, LANGUAGE_ITALIANO);
    pdata->cb_italiano = checkbox;

    btn = view_common_menu_button(lv_scr_act(), &img_return, BACK_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 24, -24);

    btn = view_common_menu_button(lv_scr_act(), &img_confirm, CONFIRM_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -24, -24);

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
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case CONFIRM_BTN_ID:
                            model_set_language(pmodel, pdata->language);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        default:
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case CB_BTN_ID:
                            pdata->language = event.data.number;
                            update_page(pmodel, pdata);
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


static void update_page(model_t *pmodel, struct page_data *pdata) {
    view_common_set_checked(pdata->cb_english, pdata->language == LANGUAGE_ENGLISH);
    view_common_set_checked(pdata->cb_italiano, pdata->language == LANGUAGE_ITALIANO);
}


static void update_time(model_t *pmodel, struct page_data *pdata) {
    time_t    time_now = time(NULL);
    struct tm tm_info  = *localtime(&time_now);

    lv_label_set_text_fmt(pdata->lbl_time, "%i.%i.%i - %02i:%02i:%02i", tm_info.tm_year + 1900, tm_info.tm_mon + 1,
                          tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
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


const pman_page_t page_language = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};