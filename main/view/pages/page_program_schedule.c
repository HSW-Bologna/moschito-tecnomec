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


LV_IMG_DECLARE(img_check_sm);
LV_IMG_DECLARE(img_return);
LV_IMG_DECLARE(img_erogator_1_sm);
LV_IMG_DECLARE(img_erogator_2_sm);
LV_IMG_DECLARE(img_time_start);
LV_IMG_DECLARE(img_time_stop);
LV_IMG_DECLARE(img_timed_red);
LV_IMG_DECLARE(img_timed_green);


enum {
    CB_BTN_ID,
    TIME_TIMER_ID,
    BACK_BTN_ID,
    DAYS_BTN_ID,
    TIME_START_BTN_ID,
    TIME_STOP_BTN_ID,
    CONTINUOUS_BTN_ID,
    TIMED_BTN_ID,
    TIMED_CONF_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_obj_t *btn_continuous;
    lv_obj_t *btn_timed;
    lv_obj_t *btn_timed_conf;

    lv_timer_t *timer_time;

    view_common_program_t *program;
};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static void      update_time(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *day_checkbox(lv_obj_t *root, const char *str, uint8_t checked);
static lv_obj_t *time_widget(lv_obj_t *root, unsigned int seconds);
static lv_obj_t *label_modify_preview(lv_obj_t *root, const char *desc, int value);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS]       = {&img_erogator_1_sm, &img_erogator_2_sm};
static const lv_color_t    color_erogators[NUM_EROGATORS]     = {STYLE_EROGATOR_1_COLOR, STYLE_EROGATOR_2_COLOR};
static const lv_img_dsc_t *img_timed_erogators[NUM_EROGATORS] = {&img_timed_red, &img_timed_green};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->program = extra;

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *img, *obj;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, img_erogators[pdata->program->erogator]);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -8, 8);

    view_common_program_label(pmodel, lv_scr_act(), color_erogators[pdata->program->erogator], pdata->program->num);

    btn = view_common_menu_button(lv_scr_act(), &img_return, BACK_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 24, 24);

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 270, 64);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 125);

    const strings_t weekdays[7] = {
        STRINGS_LUN, STRINGS_MAR, STRINGS_MER, STRINGS_GIO, STRINGS_VEN, STRINGS_SAB, STRINGS_DOM,
    };
    const scheduler_dow_t weekdays_codes[7] = {
        SCHEDULER_DOW_MONDAY, SCHEDULER_DOW_TUESDAY,  SCHEDULER_DOW_WEDNESDAY, SCHEDULER_DOW_THURSDAY,
        SCHEDULER_DOW_FRIDAY, SCHEDULER_DOW_SATURDAY, SCHEDULER_DOW_SUNDAY,
    };
    const int32_t deltas[7] = {
        -114, -76, -38, 0, 38, 76, 114,
    };
    for (size_t i = 0; i < 7; i++) {
        lv_obj_t *widget = day_checkbox(btn, view_intl_get_string(pmodel, weekdays[i]),
                                        (model_get_program_days(pmodel, pdata->program->erogator, pdata->program->num) &
                                         (1 << weekdays_codes[i])) > 0);
        lv_obj_align(widget, LV_ALIGN_CENTER, deltas[i], 0);
    }
    view_register_object_default_callback(btn, DAYS_BTN_ID);


    btn = time_widget(lv_scr_act(),
                      model_get_program_start_second(pmodel, pdata->program->erogator, pdata->program->num));
    view_register_object_default_callback(btn, TIME_START_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 190);

    img = lv_img_create(lv_scr_act());
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    lv_img_set_src(img, &img_time_start);

    lv_obj_align_to(img, btn, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    btn =
        time_widget(lv_scr_act(), model_get_program_stop_second(pmodel, pdata->program->erogator, pdata->program->num));
    view_register_object_default_callback(btn, TIME_STOP_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 25, 265);

    img = lv_img_create(lv_scr_act());
    lv_obj_add_style(img, (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    lv_img_set_src(img, &img_time_stop);

    lv_obj_align_to(img, btn, LV_ALIGN_OUT_RIGHT_MID, 20, 0);


    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 270, 125);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 345);
    view_register_object_default_callback(btn, CONTINUOUS_BTN_ID);
    pdata->btn_continuous = btn;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 270, 50);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 345);

    img = lv_img_create(btn);
    lv_img_set_src(img, img_timed_erogators[pdata->program->erogator]);
    lv_obj_center(img);

    view_register_object_default_callback(btn, TIMED_BTN_ID);
    pdata->btn_timed = btn;

    btn = lv_btn_create(lv_scr_act());
    lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 270, 70);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 400);

    obj = label_modify_preview(btn, view_intl_get_string(pmodel, STRINGS_SECONDI),
                               model_get_erogation_active_time(pmodel, pdata->program->erogator, pdata->program->num));
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 48, 4);

    obj = label_modify_preview(btn, view_intl_get_string(pmodel, STRINGS_MINUTI),
                               model_get_erogation_pause_time(pmodel, pdata->program->erogator, pdata->program->num) /
                                   60);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 48, -4);

    view_register_object_default_callback(btn, TIMED_CONF_BTN_ID);
    pdata->btn_timed_conf = btn;


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

                        case DAYS_BTN_ID: {
                            view_common_program_t *program = lv_mem_alloc(sizeof(view_common_program_t));
                            assert(program != NULL);
                            memcpy(program, pdata->program, sizeof(view_common_program_t));

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = program;
                            msg.vmsg.page  = &page_weekdays;
                            break;
                        }

                        case TIME_START_BTN_ID: {
                            view_common_program_t *program = lv_mem_alloc(sizeof(view_common_program_t));
                            assert(program != NULL);
                            memcpy(program, pdata->program, sizeof(view_common_program_t));

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = program;
                            msg.vmsg.page  = &page_program_start_time;
                            break;
                        }

                        case TIME_STOP_BTN_ID: {
                            view_common_program_t *program = lv_mem_alloc(sizeof(view_common_program_t));
                            assert(program != NULL);
                            memcpy(program, pdata->program, sizeof(view_common_program_t));

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = program;
                            msg.vmsg.page  = &page_program_stop_time;
                            break;
                        }

                        case TIMED_BTN_ID:
                        case CONTINUOUS_BTN_ID:
                            model_toggle_working_mode(pmodel, pdata->program->erogator, pdata->program->num);
                            update_page(pmodel, pdata);
                            break;

                        case TIMED_CONF_BTN_ID: {
                            view_common_program_t *program = lv_mem_alloc(sizeof(view_common_program_t));
                            assert(program != NULL);
                            memcpy(program, pdata->program, sizeof(view_common_program_t));

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = program;
                            msg.vmsg.page  = &page_timed_config;
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case CB_BTN_ID:
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
    if (model_get_working_mode(pmodel, pdata->program->erogator, pdata->program->num) == WORKING_MODE_CONTINUOUS) {
        view_common_set_hidden(pdata->btn_continuous, 0);
        view_common_set_hidden(pdata->btn_timed, 1);
        view_common_set_hidden(pdata->btn_timed_conf, 1);
    } else {
        view_common_set_hidden(pdata->btn_continuous, 1);
        view_common_set_hidden(pdata->btn_timed, 0);
        view_common_set_hidden(pdata->btn_timed_conf, 0);
    }
}


static void update_time(model_t *pmodel, struct page_data *pdata) {
    time_t    time_now = time(NULL);
    struct tm tm_info  = *localtime(&time_now);

    lv_label_set_text_fmt(pdata->lbl_time, "%i.%i.%i - %02i:%02i:%02i", tm_info.tm_year + 1900, tm_info.tm_mon + 1,
                          tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
}


static lv_obj_t *time_widget(lv_obj_t *root, unsigned int seconds) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 180, 70);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lbl, "%02i:%02i", (seconds / 3600), (seconds / 60) % 60);
    lv_obj_center(lbl);

    return btn;
}


static lv_obj_t *day_checkbox(lv_obj_t *root, const char *str, uint8_t checked) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 40, 50);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t *obj_day = lv_obj_create(cont);
    lv_obj_set_style_bg_opa(obj_day, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_add_flag(obj_day, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_style(obj_day, (lv_style_t *)&style_bordered_container, LV_STATE_DEFAULT);
    lv_obj_set_size(obj_day, 32, 20);
    lv_obj_align(obj_day, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *lbl = lv_label_create(obj_day);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_TINY, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, str);
    lv_obj_center(lbl);

    lv_obj_t *checkbox = lv_checkbox_create(cont);
    lv_checkbox_set_text(checkbox, "");
    lv_obj_set_style_bg_img_src(checkbox, &img_check_sm, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(checkbox, 2, LV_STATE_DEFAULT | LV_PART_INDICATOR);
    lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(checkbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_align(checkbox, LV_ALIGN_BOTTOM_MID, 0, 0);
    view_common_set_checked(checkbox, checked);

    return cont;
}


static lv_obj_t *label_modify_preview(lv_obj_t *root, const char *desc, int value) {
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_size(cont, 190, 25);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, 18);
    lv_label_set_text_fmt(lbl, "%i", value);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 30, 0);

    lv_obj_t *obj = view_common_modify_button(cont, LV_SYMBOL_MINUS, STYLE_FONT_SMALL, -1);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align_to(obj, lbl, LV_ALIGN_OUT_LEFT_MID, -4, 0);

    obj = view_common_modify_button(cont, LV_SYMBOL_PLUS, STYLE_FONT_SMALL, -1);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(obj, 24, 24);
    lv_obj_align_to(obj, lbl, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    lbl = lv_label_create(cont);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, 100);
    lv_label_set_text(lbl, desc);
    lv_obj_align_to(lbl, obj, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

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


const pman_page_t page_program_schedule = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};