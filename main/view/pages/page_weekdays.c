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


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
    DAY_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;
    lv_obj_t *btn_days[7];

    lv_timer_t *timer_time;

    view_common_program_t *program;
    uint8_t                days;
};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static void      update_time(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *day_btn(lv_obj_t *root, erogator_t erogator, const char *str);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS]   = {&img_erogator_1_sm, &img_erogator_2_sm};
static const lv_color_t    color_erogators[NUM_EROGATORS] = {STYLE_EROGATOR_1_COLOR, STYLE_EROGATOR_2_COLOR};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->program = extra;
    pdata->days    = model_get_program_days(pmodel, pdata->program->erogator, pdata->program->num);

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
    lv_img_set_src(img, img_erogators[pdata->program->erogator]);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -8, 8);

    view_common_program_label(pmodel, lv_scr_act(), color_erogators[pdata->program->erogator], pdata->program->num);

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_LUN));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_MONDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 48);
    pdata->btn_days[SCHEDULER_DOW_MONDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_MAR));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_TUESDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 120);
    pdata->btn_days[SCHEDULER_DOW_TUESDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_MER));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_WEDNESDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -25, 120);
    pdata->btn_days[SCHEDULER_DOW_WEDNESDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_GIO));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_THURSDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 190);
    pdata->btn_days[SCHEDULER_DOW_THURSDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_VEN));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_FRIDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -25, 190);
    pdata->btn_days[SCHEDULER_DOW_FRIDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_SAB));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_SATURDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 260);
    pdata->btn_days[SCHEDULER_DOW_SATURDAY] = btn;

    btn = day_btn(lv_scr_act(), pdata->program->erogator, view_intl_get_string(pmodel, STRINGS_DOM));
    view_register_object_default_callback_with_number(btn, DAY_BTN_ID, SCHEDULER_DOW_SUNDAY);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -25, 260);
    pdata->btn_days[SCHEDULER_DOW_SUNDAY] = btn;


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
                            model_set_program_days(pmodel, pdata->program->erogator, pdata->program->num, pdata->days);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case DAY_BTN_ID:
                            if ((pdata->days & (1 << event.data.number)) > 0) {
                                pdata->days &= ~(1 << event.data.number);
                            } else {
                                pdata->days |= 1 << event.data.number;
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
    for (size_t i = 0; i < 7; i++) {
        view_common_set_checked(pdata->btn_days[i], (pdata->days & (1 << i)) > 0);
    }
}


static void update_time(model_t *pmodel, struct page_data *pdata) {
    time_t    time_now = time(NULL);
    struct tm tm_info  = *localtime(&time_now);

    lv_label_set_text_fmt(pdata->lbl_time, "%i.%i.%i - %02i:%02i:%02i", tm_info.tm_year + 1900, tm_info.tm_mon + 1,
                          tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
}


static lv_obj_t *day_btn(lv_obj_t *root, erogator_t erogator, const char *str) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_style_border_color(btn, color_erogators[erogator], LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(btn, STYLE_BG_COLOR, LV_STATE_CHECKED);
    lv_obj_set_style_border_width(btn, 4, LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btn, color_erogators[erogator], LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 4, LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btn, color_erogators[erogator], LV_STATE_CHECKED);
    lv_obj_set_style_text_color(btn, color_erogators[erogator], LV_STATE_PRESSED);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn, 100, 60);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_remove_style_all(lbl);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_BIG, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, str);
    lv_obj_center(lbl);

    return btn;
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


const pman_page_t page_weekdays = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};