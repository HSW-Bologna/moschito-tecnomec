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


LV_IMG_DECLARE(img_return);
LV_IMG_DECLARE(img_confirm);
LV_IMG_DECLARE(img_brightness_md);
LV_IMG_DECLARE(img_audio_md);
LV_IMG_DECLARE(img_disabled);


enum {
    BRIGHTNESS_MINUS_BTN_ID,
    BRIGHTNESS_PLUS_BTN_ID,
    VOLUME_MINUS_BTN_ID,
    VOLUME_PLUS_BTN_ID,
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *img_disabled;

    lv_timer_t *timer_time;

    uint8_t brightness;
    uint8_t volume;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->brightness = model_get_brightness(pmodel);
    pdata->volume     = model_get_volume(pmodel);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *obj, *img;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 32, 32);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_DISPLAY));

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 32, 190);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_VOLUME));


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 100);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 64);

    img = lv_img_create(obj);
    lv_img_set_src(img, &img_brightness_md);
    lv_obj_center(img);

    btn = view_common_modify_button(obj, LV_SYMBOL_MINUS, STYLE_FONT_HUGE, BRIGHTNESS_MINUS_BTN_ID);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 4, 0);

    btn = view_common_modify_button(obj, LV_SYMBOL_PLUS, STYLE_FONT_HUGE, BRIGHTNESS_PLUS_BTN_ID);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -4, 0);


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 100);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 224);

    img = lv_img_create(obj);
    lv_img_set_src(img, &img_audio_md);
    lv_obj_center(img);

    img = lv_img_create(obj);
    lv_img_set_src(img, &img_disabled);
    lv_obj_center(img);
    pdata->img_disabled = img;

    btn = view_common_modify_button(obj, LV_SYMBOL_MINUS, STYLE_FONT_HUGE, VOLUME_MINUS_BTN_ID);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 4, 0);

    btn = view_common_modify_button(obj, LV_SYMBOL_PLUS, STYLE_FONT_HUGE, VOLUME_PLUS_BTN_ID);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -4, 0);


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
                            model_set_brightness(pmodel, pdata->brightness);
                            model_set_volume(pmodel, pdata->volume);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case VOLUME_MINUS_BTN_ID:
                        case VOLUME_PLUS_BTN_ID:
                            pdata->volume = !pdata->volume;
                            update_page(pmodel, pdata);
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


static void update_page(model_t *pmodel, struct page_data *pdata) {
    view_common_set_hidden(pdata->img_disabled, pdata->volume);
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


const pman_page_t page_display = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};