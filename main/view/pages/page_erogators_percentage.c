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
    EROGATOR_1_MINUS_BTN_ID,
    EROGATOR_1_PLUS_BTN_ID,
    EROGATOR_2_MINUS_BTN_ID,
    EROGATOR_2_PLUS_BTN_ID,
    TIME_TIMER_ID,
    BACK_BTN_ID,
    CONFIRM_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;

    lv_timer_t *timer_time;
    lv_obj_t   *lbl_erogators[NUM_EROGATORS];

    uint8_t erogator_percentages[NUM_EROGATORS];
    uint8_t volume;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);
static void modify(struct page_data *pdata, erogator_t erogator, int mod);


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->erogator_percentages[EROGATOR_1] = model_get_erogator_percentage(pmodel, EROGATOR_1);
    pdata->erogator_percentages[EROGATOR_2] = model_get_erogator_percentage(pmodel, EROGATOR_2);

    return pdata;
}


static void open_page(model_t *pmodel, void *args) {
    struct page_data *pdata = args;
    lv_obj_t         *btn, *lbl, *obj, *img;

    lv_timer_resume(pdata->timer_time);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 4);
    pdata->lbl_time = lbl;

    obj = view_common_horizontal_parameter_widget(lv_scr_act(), &pdata->lbl_erogators[EROGATOR_1],
                                                  EROGATOR_1_MINUS_BTN_ID, EROGATOR_1_PLUS_BTN_ID);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, -100);

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_erogator_1_sm);
    lv_obj_align_to(img, obj, LV_ALIGN_OUT_TOP_RIGHT,-4,0);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "%");
    lv_obj_align_to(lbl, obj, LV_ALIGN_TOP_MID, 0, -32);

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 140, 24);
    lv_obj_set_style_bg_color(cont, STYLE_EROGATOR_1_COLOR, LV_STATE_DEFAULT);

    lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lbl, "%s 1", view_intl_get_string(pmodel, STRINGS_PRODOTTO));
    lv_obj_center(lbl);

    lv_obj_align_to(cont, obj, LV_ALIGN_TOP_MID, 0, -64);


    obj = view_common_horizontal_parameter_widget(lv_scr_act(), &pdata->lbl_erogators[EROGATOR_2],
                                                  EROGATOR_2_MINUS_BTN_ID, EROGATOR_2_PLUS_BTN_ID);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 80);

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_erogator_2_sm);
    lv_obj_align_to(img, obj, LV_ALIGN_OUT_TOP_RIGHT,8,0);

    lbl = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(lbl, STYLE_FONT_HUGE, LV_STATE_DEFAULT);
    lv_label_set_text(lbl, "%");
    lv_obj_align_to(lbl, obj, LV_ALIGN_TOP_MID, 0, -32);

    cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 140, 24);
    lv_obj_set_style_bg_color(cont, STYLE_EROGATOR_2_COLOR, LV_STATE_DEFAULT);

    lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, STYLE_FONT_SMALL, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(lbl, "%s 2", view_intl_get_string(pmodel, STRINGS_PRODOTTO));
    lv_obj_center(lbl);

    lv_obj_align_to(cont, obj, LV_ALIGN_TOP_MID, 0, -64);

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
                case LV_EVENT_LONG_PRESSED_REPEAT:
                case LV_EVENT_CLICKED: {
                    switch (event.data.id) {
                        case BACK_BTN_ID:
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case CONFIRM_BTN_ID:
                            model_set_erogator_percentage(pmodel, EROGATOR_1, pdata->erogator_percentages[EROGATOR_1]);
                            model_set_erogator_percentage(pmodel, EROGATOR_2, pdata->erogator_percentages[EROGATOR_2]);
                            msg.vmsg.code = VIEW_PAGE_MESSAGE_CODE_BACK;
                            break;

                        case EROGATOR_1_MINUS_BTN_ID:
                            modify(pdata, EROGATOR_1, -1);
                            update_page(pmodel, pdata);
                            break;

                        case EROGATOR_1_PLUS_BTN_ID:
                            modify(pdata, EROGATOR_1, +1);
                            update_page(pmodel, pdata);
                            break;

                        case EROGATOR_2_MINUS_BTN_ID:
                            modify(pdata, EROGATOR_2, -1);
                            update_page(pmodel, pdata);
                            break;

                        case EROGATOR_2_PLUS_BTN_ID:
                            modify(pdata, EROGATOR_2, +1);
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
    lv_label_set_text_fmt(pdata->lbl_erogators[EROGATOR_1], "%i", pdata->erogator_percentages[EROGATOR_1]);
    lv_label_set_text_fmt(pdata->lbl_erogators[EROGATOR_2], "%i", pdata->erogator_percentages[EROGATOR_2]);
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


static void modify(struct page_data *pdata, erogator_t erogator, int mod) {
    int value = pdata->erogator_percentages[erogator];
    value += mod;

    if (value < 0) {
        value = 0;
    } else if (value > 100) {
        value = 100;
    }

    pdata->erogator_percentages[erogator] = value;
}


const pman_page_t page_erogators_percentage = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};