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
LV_IMG_DECLARE(img_date);


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
    DAY_MINUS_BTN_ID,
    DAY_PLUS_BTN_ID,
    MONTH_MINUS_BTN_ID,
    MONTH_PLUS_BTN_ID,
    YEAR_MINUS_BTN_ID,
    YEAR_PLUS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *lbl_day;
    lv_obj_t *lbl_month;
    lv_obj_t *lbl_year;

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
    lv_img_set_src(img, &img_date);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32, 32);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_MEDIUM, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 32, 102);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_GIORNO));


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 240);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 132);

    lv_obj_t *day = view_common_vertical_parameter_widget(obj, &pdata->lbl_day, DAY_MINUS_BTN_ID, DAY_PLUS_BTN_ID);
    lv_obj_align(day, LV_ALIGN_CENTER, -90, 0);

    lv_obj_t *month =
        view_common_vertical_parameter_widget(obj, &pdata->lbl_month, MONTH_MINUS_BTN_ID, MONTH_PLUS_BTN_ID);
    lv_obj_align(month, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *year = view_common_vertical_parameter_widget(obj, &pdata->lbl_year, YEAR_MINUS_BTN_ID, YEAR_PLUS_BTN_ID);
    lv_obj_align(year, LV_ALIGN_CENTER, 90, 0);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "/");
    lv_obj_align(lbl, LV_ALIGN_CENTER, -45, 12);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "/");
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
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case DAY_MINUS_BTN_ID:
                            pdata->time_info.tm_mday--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case DAY_PLUS_BTN_ID:
                            pdata->time_info.tm_mday++;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case MONTH_MINUS_BTN_ID:
                            pdata->time_info.tm_mon--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case MONTH_PLUS_BTN_ID:
                            pdata->time_info.tm_mon++;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case YEAR_MINUS_BTN_ID:
                            pdata->time_info.tm_year--;
                            mktime(&pdata->time_info);
                            update_page(pmodel, pdata);
                            break;

                        case YEAR_PLUS_BTN_ID:
                            pdata->time_info.tm_year++;
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
    lv_label_set_text_fmt(pdata->lbl_day, "%02i", pdata->time_info.tm_mday);
    lv_label_set_text_fmt(pdata->lbl_month, "%02i", pdata->time_info.tm_mon + 1);
    lv_label_set_text_fmt(pdata->lbl_year, "%02i", pdata->time_info.tm_year - 100);
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


const pman_page_t page_date = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};