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


LV_IMG_DECLARE(img_stop_active);
LV_IMG_DECLARE(img_stop_inactive);
LV_IMG_DECLARE(img_gear);
LV_IMG_DECLARE(img_sun);
LV_IMG_DECLARE(img_moon);
LV_IMG_DECLARE(img_rondine);
LV_IMG_DECLARE(img_erogators_inactive);
LV_IMG_DECLARE(img_erogators_1);
LV_IMG_DECLARE(img_erogators_2);
LV_IMG_DECLARE(img_erogation_1);
LV_IMG_DECLARE(img_erogation_2);
LV_IMG_DECLARE(img_dead_mosquito);
LV_IMG_DECLARE(img_live_mosquito);


enum {
    STOP_TOGGLE_BTN_ID,
    SETTINGS_BTN_ID,
    NIGHT_BTN_ID,
    DAY_BTN_ID,
    TIME_TIMER_ID,
};


struct page_data {
    lv_obj_t *img_stop;

    view_common_erogator_graphic_t erogator_objs;

    lv_obj_t *lbl_time;

    lv_timer_t *timer_time;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *img;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_rondine);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 32);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 56, 56);
    img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, &img_gear);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_add_style(btn, (lv_style_t *)&style_icon_btn, LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, SETTINGS_BTN_ID);

    view_common_erogator_graphic_create(lv_scr_act(), &pdata->erogator_objs);

    btn = view_common_menu_button(lv_scr_act(), &img_moon, NIGHT_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 106, 90);
    img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, &img_stop_active);
    pdata->img_stop = img;
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -5);
    view_register_object_default_callback(btn, STOP_TOGGLE_BTN_ID);

    btn = view_common_menu_button(lv_scr_act(), &img_sun, DAY_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

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
                        case STOP_TOGGLE_BTN_ID:
                            model_toggle_stop(pmodel);
                            update_page(pmodel, pdata);
                            break;

                        case SETTINGS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_settings;
                            break;

                        case NIGHT_BTN_ID:
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_selection;
                            msg.vmsg.extra = (void *)(uintptr_t)EROGATOR_1;
                            break;

                        case DAY_BTN_ID:
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_selection;
                            msg.vmsg.extra = (void *)(uintptr_t)EROGATOR_2;
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
    view_common_img_set_src(pdata->img_stop, model_get_stop(pmodel) ? &img_stop_active : &img_stop_inactive);
    view_common_update_erogator_graphic(&pdata->erogator_objs, model_get_erogators_state(pmodel));
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


const pman_page_t page_main = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};