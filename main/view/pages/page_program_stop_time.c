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
#include "config/app_config.h"


LV_IMG_DECLARE(img_return);
LV_IMG_DECLARE(img_confirm);
LV_IMG_DECLARE(img_time_stop);
LV_IMG_DECLARE(img_erogator_1_sm);
LV_IMG_DECLARE(img_erogator_2_sm);


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
    HOUR_MINUS_BTN_ID,
    HOUR_PLUS_BTN_ID,
    MINUTE_MINUS_BTN_ID,
    MINUTE_PLUS_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *lbl_hour;
    lv_obj_t *lbl_minute;

    lv_timer_t *timer_time;

    view_common_program_t *program;
    unsigned long          stop_time;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS]   = {&img_erogator_1_sm, &img_erogator_2_sm};
static const lv_color_t    color_erogators[NUM_EROGATORS] = {STYLE_EROGATOR_1_COLOR, STYLE_EROGATOR_2_COLOR};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->program   = extra;
    pdata->stop_time = model_get_program_stop_second(pmodel, pdata->program->erogator, pdata->program->num);

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
    lv_img_set_src(img, img_erogators[pdata->program->erogator]);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -8, 8);

    view_common_program_label(pmodel, lv_scr_act(), color_erogators[pdata->program->erogator], pdata->program->num);

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_time_stop);
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32, 32);

    obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj, 270, 240);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 132);

    lv_obj_t *hour = view_common_vertical_parameter_widget(obj, &pdata->lbl_hour, HOUR_MINUS_BTN_ID, HOUR_PLUS_BTN_ID);
    lv_obj_align(hour, LV_ALIGN_CENTER, -70, 0);

    lv_obj_t *minute =
        view_common_vertical_parameter_widget(obj, &pdata->lbl_minute, MINUTE_MINUS_BTN_ID, MINUTE_PLUS_BTN_ID);
    lv_obj_align(minute, LV_ALIGN_CENTER, 70, 0);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, ":");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);


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

    unsigned long start_time = model_get_program_start_second(pmodel, pdata->program->erogator, pdata->program->num);

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
                            model_set_program_stop_second(pmodel, pdata->program->erogator, pdata->program->num,
                                                          pdata->stop_time);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case HOUR_MINUS_BTN_ID:
                            if (pdata->stop_time > start_time + 3600 ||
                                model_get_working_mode(pmodel, pdata->program->erogator, pdata->program->num) ==
                                    WORKING_MODE_TIMED) {
                                if (pdata->stop_time > 3600) {
                                    pdata->stop_time -= 3600;
                                } else {
                                    pdata->stop_time = 0;
                                }
                            } else {
                                pdata->stop_time = start_time;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case HOUR_PLUS_BTN_ID:
                            if (pdata->stop_time < 3600 * 23) {
                                pdata->stop_time += 3600;
                            } else {
                                pdata->stop_time = 3600 * 24 - 1;
                            }

                            if (pdata->stop_time > start_time + APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS &&
                                model_get_working_mode(pmodel, pdata->program->erogator, pdata->program->num) ==
                                    WORKING_MODE_CONTINUOUS) {
                                pdata->stop_time = start_time + APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS;
                            }

                            update_page(pmodel, pdata);
                            break;

                        case MINUTE_MINUS_BTN_ID:
                            if (pdata->stop_time > start_time + 60 ||
                                model_get_working_mode(pmodel, pdata->program->erogator, pdata->program->num)) {
                                if (pdata->stop_time > 60) {
                                    pdata->stop_time -= 60;
                                } else {
                                    pdata->stop_time = 0;
                                }
                            } else {
                                pdata->stop_time = start_time;
                            }
                            update_page(pmodel, pdata);
                            break;

                        case MINUTE_PLUS_BTN_ID:
                            if (pdata->stop_time < 3600 * 24 - 60) {
                                pdata->stop_time += 60;
                            } else {
                                pdata->stop_time = 3600 * 24 - 1;
                            }

                            if (pdata->stop_time > start_time + APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS &&
                                model_get_working_mode(pmodel, pdata->program->erogator, pdata->program->num) ==
                                    WORKING_MODE_CONTINUOUS) {
                                pdata->stop_time = start_time + APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS;
                            }

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
    lv_label_set_text_fmt(pdata->lbl_hour, "%02lu", pdata->stop_time / 3600);
    lv_label_set_text_fmt(pdata->lbl_minute, "%02lu", (pdata->stop_time / 60) % 60);
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


const pman_page_t page_program_stop_time = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};