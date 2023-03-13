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


LV_IMG_DECLARE(img_gear);
LV_IMG_DECLARE(img_home_icon);
LV_IMG_DECLARE(img_manual_md);
LV_IMG_DECLARE(img_auto_md);
LV_IMG_DECLARE(img_sun);
LV_IMG_DECLARE(img_moon);


enum {
    SETTINGS_BTN_ID,
    HOME_BTN_ID,
    MANUAL_BTN_ID,
    AUTOMATIC_BTN_ID,
    TIME_TIMER_ID,
};


struct page_data {
    view_common_erogator_graphic_t erogator_objs;

    lv_obj_t *lbl_time;

    lv_obj_t *img_sun;
    lv_obj_t *img_moon;

    erogator_t erogator;

    lv_timer_t *timer_time;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    assert(pdata != NULL);
    pdata->timer_time = view_register_periodic_timer(500UL, TIME_TIMER_ID);
    pdata->erogator   = (erogator_t)(uintptr_t)extra;
    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *img;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 56, 56);
    img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, &img_gear);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_add_style(btn, (lv_style_t *)&style_icon_btn, LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, SETTINGS_BTN_ID);

    view_common_erogator_graphic_create(lv_scr_act(), &pdata->erogator_objs);
    lv_obj_align(pdata->erogator_objs.img_erogators, LV_ALIGN_CENTER, 0, -8);
    view_common_erogator_graphic_realign(&pdata->erogator_objs);

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_moon);
    lv_img_set_zoom(img, 128 + 64);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 16, 0);
    pdata->img_moon = img;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_sun);
    lv_img_set_zoom(img, 128 + 64);
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, -16, 0);
    pdata->img_sun = img;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 64, 64);
    img = lv_img_create(btn);
    lv_obj_center(img);
    lv_img_set_src(img, &img_home_icon);
    lv_obj_add_style(btn, (lv_style_t *)&style_icon_btn, LV_STATE_DEFAULT);
    view_register_object_default_callback(btn, HOME_BTN_ID);

    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 20);

    btn = view_common_menu_button(lv_scr_act(), &img_manual_md, MANUAL_BTN_ID);
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 32, -8);

    btn = view_common_menu_button(lv_scr_act(), &img_auto_md, AUTOMATIC_BTN_ID);
    lv_obj_set_size(btn, 100, 100);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -32, -8);

    update_page(pmodel, pdata);
    update_time(pmodel, pdata);
}


static view_message_t page_event(model_t *pmodel, void *args, view_event_t event) {
    view_message_t    msg   = VIEW_NULL_MESSAGE;
    struct page_data *pdata = args;

    switch (event.code) {
        case VIEW_EVENT_CODE_TIMER:
            update_time(pmodel, pdata);
            break;

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

                        case SETTINGS_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_settings;
                            break;

                        case MANUAL_BTN_ID:
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_manual;
                            msg.vmsg.extra = (void *)(uintptr_t)pdata->erogator;
                            break;

                        case AUTOMATIC_BTN_ID:
                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_auto;
                            msg.vmsg.extra = (void *)(uintptr_t)pdata->erogator;
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
    erogators_state_t corrisponding_state = pdata->erogator == EROGATOR_1 ? EROGATORS_STATE_1 : EROGATORS_STATE_2;

    view_common_update_erogator_graphic(&pdata->erogator_objs, corrisponding_state, 0, 0, 0);

    view_common_set_hidden(pdata->img_moon, pdata->erogator != EROGATOR_1);
    view_common_set_hidden(pdata->img_sun, pdata->erogator != EROGATOR_2);
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
}


const pman_page_t page_selection = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};