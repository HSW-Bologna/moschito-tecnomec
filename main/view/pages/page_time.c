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
LV_IMG_DECLARE(img_time);


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
    HOUR_MINUS_BTN_ID,
    HOUR_PLUS_BTN_ID,
    MINUTE_MINUS_BTN_ID,
    MINUTE_PLUS_BTN_ID,
    SECOND_MINUS_BTN_ID,
    SECOND_PLUS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *lbl_hour;
    lv_obj_t *lbl_minute;
    lv_obj_t *lbl_second;

    lv_timer_t *timer_time;

    struct tm time_info;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    time_t time_now  = time(NULL);
    pdata->time_info = *localtime(&time_now);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *obj, *img;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_time);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32, 32);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 32, 102);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_ORA));


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 240);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 132);

    lv_obj_t *day = view_common_vertical_parameter_widget(obj, &pdata->lbl_hour, HOUR_MINUS_BTN_ID, HOUR_PLUS_BTN_ID);
    lv_obj_align(day, LV_ALIGN_CENTER, -90, 0);

    lv_obj_t *month =
        view_common_vertical_parameter_widget(obj, &pdata->lbl_minute, MINUTE_MINUS_BTN_ID, MINUTE_PLUS_BTN_ID);
    lv_obj_align(month, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *year =
        view_common_vertical_parameter_widget(obj, &pdata->lbl_second, SECOND_MINUS_BTN_ID, SECOND_PLUS_BTN_ID);
    lv_obj_align(year, LV_ALIGN_CENTER, 90, 0);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, ":");
    lv_obj_align(lbl, LV_ALIGN_CENTER, -45, 12);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, ":");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 45, 12);


    btn = view_common_menu_button(lv_scr_act(), &img_return, BACK_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 24, -8);

    btn = view_common_menu_button(lv_scr_act(), &img_confirm, CONFIRM_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -24, -8);

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
                            msg.vmsg.code      = VIEW_PAGE_MESSAGE_CODE_BACK;
                            msg.cmsg.code      = VIEW_CONTROLLER_MESSAGE_CODE_SAVE_RTC_TIME;
                            msg.cmsg.time_info = pdata->time_info;
                            break;

                        case HOUR_MINUS_BTN_ID:
                            pdata->time_info.tm_hour--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case HOUR_PLUS_BTN_ID:
                            pdata->time_info.tm_hour++;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case MINUTE_MINUS_BTN_ID:
                            pdata->time_info.tm_min--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case MINUTE_PLUS_BTN_ID:
                            pdata->time_info.tm_min++;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case SECOND_MINUS_BTN_ID:
                            pdata->time_info.tm_sec--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case SECOND_PLUS_BTN_ID:
                            pdata->time_info.tm_sec++;
                            mktime(&pdata->time_info);
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
    lv_label_set_text_fmt(pdata->lbl_hour, "%02i", pdata->time_info.tm_hour);
    lv_label_set_text_fmt(pdata->lbl_minute, "%02i", pdata->time_info.tm_min);
    lv_label_set_text_fmt(pdata->lbl_second, "%02i", pdata->time_info.tm_sec);
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


const pman_page_t page_time = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};