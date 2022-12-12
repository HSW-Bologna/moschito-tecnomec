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
LV_IMG_DECLARE(img_home_icon);
LV_IMG_DECLARE(img_erogator_1_sm);
LV_IMG_DECLARE(img_erogator_2_sm);
LV_IMG_DECLARE(img_bin);
LV_IMG_DECLARE(img_confirm);


enum {
    TIME_TIMER_ID,
    BACK_BTN_ID,
};


struct page_data {
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_program[3];
    lv_obj_t *cb_program[3];

    lv_timer_t *timer_time;

    uint8_t    page_index;
    erogator_t erogator;
};


static void update_page(model_t *pmodel, struct page_data *pdata);
static void update_time(model_t *pmodel, struct page_data *pdata);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS] = {&img_erogator_1_sm, &img_erogator_2_sm};


static void *create_page(model_t *pmodel, void *extra) {
    struct page_data *pdata = malloc(sizeof(struct page_data));
    pdata->timer_time       = view_register_periodic_timer(500UL, TIME_TIMER_ID);

    pdata->page_index = 0;

    pdata->erogator = (erogator_t)(uintptr_t)extra;

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
    lv_img_set_src(img, img_erogators[pdata->erogator]);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -8, 8);

    obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 130, 72);
    lv_obj_add_style(obj, (lv_style_t *)&style_border, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT);

    lbl = lv_label_create(obj);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl, 122);
    lv_obj_center(lbl);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_MULTI_PROGRAMMA));

    lv_obj_align(obj, LV_ALIGN_TOP_MID, 24, 40);

    obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 270, 300);
    lv_obj_add_style(obj, (lv_style_t *)&style_border, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    lbl = lv_label_create(obj);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, 254);
    lv_label_set_text(lbl, view_intl_get_string(pmodel, STRINGS_L_ORARIO_DI_PROGRAMMAZIONE_COINCIDE));
    lv_obj_center(lbl);

    btn = view_common_menu_button(lv_scr_act(), &img_return, BACK_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 24, -24);

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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


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


const pman_page_t page_warning = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};