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
LV_IMG_DECLARE(img_erogator_1_sm);
LV_IMG_DECLARE(img_erogator_2_sm);
LV_IMG_DECLARE(img_timed_red);
LV_IMG_DECLARE(img_timed_green);


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
    ACTIVE_TIME_MINUS_BTN_ID,
    ACTIVE_TIME_PLUS_BTN_ID,
    PAUSE_TIME_MINUS_BTN_ID,
    PAUSE_TIME_PLUS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_active_time;
    lv_obj_t *lbl_pause_time;

    lv_obj_t *img_disabled;

    lv_timer_t *timer_time;

    view_common_program_t *program;

    unsigned long active_time;
    unsigned long pause_time;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS]       = {&img_erogator_1_sm, &img_erogator_2_sm};
static const lv_color_t    color_erogators[NUM_EROGATORS]     = {STYLE_EROGATOR_1_COLOR, STYLE_EROGATOR_2_COLOR};
static const lv_img_dsc_t *img_timed_erogators[NUM_EROGATORS] = {&img_timed_red, &img_timed_green};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->program = extra;

    pdata->active_time = model_get_erogation_active_time(pmodel, pdata->program->erogator, pdata->program->num);
    pdata->pause_time  = model_get_erogation_pause_time(pmodel, pdata->program->erogator, pdata->program->num);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *obj, *img, *widget;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, img_erogators[pdata->program->erogator]);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -8, 8);

    view_common_program_label(pmodel, lv_scr_act(), color_erogators[pdata->program->erogator], pdata->program->num);

    obj = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 220, 40);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 25, 60);

    img = lv_img_create(obj);
    lv_img_set_src(img, img_timed_erogators[pdata->program->erogator]);
    lv_img_set_zoom(img, 190);
    lv_obj_center(img);


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 120);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 110);

    widget = view_common_horizontal_parameter_widget(obj, &pdata->lbl_active_time, ACTIVE_TIME_MINUS_BTN_ID,
                                                     ACTIVE_TIME_PLUS_BTN_ID);
    lv_obj_align(widget, LV_ALIGN_BOTTOM_MID, 0, -4);

    lbl = lv_label_create(obj);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "Sec.");
    lv_obj_align_to(lbl, widget, LV_ALIGN_OUT_TOP_RIGHT, -8, 0);


    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 120);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 240);

    widget = view_common_horizontal_parameter_widget(obj, &pdata->lbl_pause_time, PAUSE_TIME_MINUS_BTN_ID,
                                                     PAUSE_TIME_PLUS_BTN_ID);
    lv_obj_align(widget, LV_ALIGN_BOTTOM_MID, 0, -4);

    lbl = lv_label_create(obj);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "Min.");
    lv_obj_align_to(lbl, widget, LV_ALIGN_OUT_TOP_RIGHT, -8, 0);


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

                        case ACTIVE_TIME_MINUS_BTN_ID:
                            if (pdata->active_time > 1) {
                                pdata->active_time--;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case ACTIVE_TIME_PLUS_BTN_ID:
                            if (pdata->active_time < 60) {
                                pdata->active_time++;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case PAUSE_TIME_MINUS_BTN_ID:
                            if (pdata->pause_time > 60) {
                                pdata->pause_time -= 60;
                            } else {
                                pdata->pause_time = 60;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case PAUSE_TIME_PLUS_BTN_ID:
                            if (pdata->pause_time < 60 * 30) {
                                pdata->pause_time += 60;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case CONFIRM_BTN_ID:
                            model_set_erogation_active_time(pmodel, pdata->program->erogator, pdata->program->num,
                                                            pdata->active_time);
                            model_set_erogation_pause_time(pmodel, pdata->program->erogator, pdata->program->num,
                                                           pdata->pause_time);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
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
    lv_label_set_text_fmt(pdata->lbl_active_time, "%lu", pdata->active_time);
    lv_label_set_text_fmt(pdata->lbl_pause_time, "%lu", pdata->pause_time / 60);
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


const pman_page_t page_timed_config = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};