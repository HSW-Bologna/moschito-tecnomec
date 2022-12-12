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
    ERASE_BTN_ID,
    NEXT_BTN_ID,
    SELECT_PROG_BTN_ID,
    CHECK_PROG_CB_ID,
};


struct page_data {
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_program[3];
    lv_obj_t *cb_program[3];

    lv_obj_t *img_next;

    lv_timer_t *timer_time;

    uint8_t    page_index;
    erogator_t erogator;
};


static void      update_page(model_t *pmodel, struct page_data *pdata);
static void      update_time(model_t *pmodel, struct page_data *pdata);
static lv_obj_t *program_editor(model_t *pmodel, lv_obj_t *root, lv_obj_t **lbl, lv_obj_t **checkbox,
                                erogator_t erogator, int btn_id, int checkbox_id, int number);


static const lv_img_dsc_t *img_erogators[NUM_EROGATORS]   = {&img_erogator_1_sm, &img_erogator_2_sm};
static const lv_color_t    color_erogators[NUM_EROGATORS] = {STYLE_EROGATOR_1_COLOR, STYLE_EROGATOR_2_COLOR};


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

    btn = view_common_menu_button(lv_scr_act(), &img_bin, ERASE_BTN_ID);
    lv_obj_add_style(lv_obj_get_child(btn, 0), (lv_style_t *)&style_alpha_icon, LV_STATE_DEFAULT);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 24, 32);

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

    lv_obj_t *editor = program_editor(pmodel, lv_scr_act(), &pdata->lbl_program[0], &pdata->cb_program[0],
                                      pdata->erogator, SELECT_PROG_BTN_ID, CHECK_PROG_CB_ID, 0);
    lv_obj_align(editor, LV_ALIGN_CENTER, 0, -80);

    editor = program_editor(pmodel, lv_scr_act(), &pdata->lbl_program[1], &pdata->cb_program[1], pdata->erogator,
                            SELECT_PROG_BTN_ID, CHECK_PROG_CB_ID, 1);
    lv_obj_align(editor, LV_ALIGN_CENTER, 0, 0);

    editor = program_editor(pmodel, lv_scr_act(), &pdata->lbl_program[2], &pdata->cb_program[2], pdata->erogator,
                            SELECT_PROG_BTN_ID, CHECK_PROG_CB_ID, 2);
    lv_obj_align(editor, LV_ALIGN_CENTER, 0, 80);

    btn = view_common_menu_button(lv_scr_act(), &img_home_icon, BACK_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 24, -24);

    btn = view_common_menu_button(lv_scr_act(), &img_return, NEXT_BTN_ID);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -24, -24);
    pdata->img_next = lv_obj_get_child(btn, 0);

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

                        case NEXT_BTN_ID:
                            pdata->page_index = !pdata->page_index;
                            update_page(pmodel, pdata);
                            break;

                        case ERASE_BTN_ID:
                            model_clear_all_programs(pmodel, pdata->erogator);
                            update_page(pmodel, pdata);
                            break;

                        case SELECT_PROG_BTN_ID: {
                            view_common_program_t *program = lv_mem_alloc(sizeof(view_common_program_t));
                            assert(program != NULL);
                            program->erogator = pdata->erogator;
                            program->num      = pdata->page_index * 3 + event.data.number;

                            msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                            msg.vmsg.page  = &page_program_schedule;
                            msg.vmsg.extra = program;
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.data.id) {
                        case CHECK_PROG_CB_ID:
                            if (model_toggle_program(pmodel, pdata->erogator,
                                                     pdata->page_index * 3 + event.data.number) < 0) {
                                msg.vmsg.code  = VIEW_PAGE_MESSAGE_CODE_CHANGE_PAGE_EXTRA;
                                msg.vmsg.extra = (void *)(uintptr_t)pdata->erogator;
                                msg.vmsg.page  = &page_warning;
                            } else {
                                update_page(pmodel, pdata);
                            }
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
    lv_img_set_angle(pdata->img_next, pdata->page_index ? 0 : 1800);

    for (size_t i = 0; i < 3; i++) {
        size_t program = 3 * pdata->page_index + i;

        lv_label_set_text_fmt(pdata->lbl_program[i], "%s %zu", view_intl_get_string(pmodel, STRINGS_PROGRAMMA),
                              program + 1);
        view_common_set_checked(pdata->cb_program[i], model_is_program_enabled(pmodel, pdata->erogator, program));
    }
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


static lv_obj_t *program_editor(model_t *pmodel, lv_obj_t *root, lv_obj_t **lbl, lv_obj_t **checkbox,
                                erogator_t erogator, int btn_id, int checkbox_id, int number) {
    lv_obj_t *obj = lv_obj_create(root);
    lv_obj_set_size(obj, 270, 80);

    lv_obj_t *btn = lv_btn_create(obj);
    lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 180, 70);

    lv_obj_t *cont = lv_obj_create(btn);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_radius(cont, 4, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 150, 25);
    lv_obj_set_style_bg_color(cont, color_erogators[erogator], LV_STATE_DEFAULT);

    *lbl = lv_label_create(cont);
    lv_obj_center(*lbl);

    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 4);

    cont = lv_obj_create(btn);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_size(cont, 100, 25);
    lv_obj_set_style_radius(cont, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cont, STYLE_FG_COLOR, LV_STATE_DEFAULT);

    lv_obj_t *lbl_edit = lv_label_create(cont);
    lv_obj_set_style_text_color(lbl_edit, STYLE_BG_COLOR, LV_STATE_DEFAULT);
    lv_label_set_text(lbl_edit, view_intl_get_string(pmodel, STRINGS_MODIFICA));
    lv_obj_center(lbl_edit);

    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);

    view_register_object_default_callback_with_number(btn, btn_id, number);

    *checkbox = lv_checkbox_create(obj);
    lv_checkbox_set_text(*checkbox, "");
    lv_obj_set_style_bg_img_src(*checkbox, &img_confirm, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(*checkbox, 26, LV_STATE_DEFAULT | LV_PART_INDICATOR);
    lv_obj_align(*checkbox, LV_ALIGN_RIGHT_MID, 0, 0);
    view_register_object_default_callback_with_number(*checkbox, checkbox_id, number);

    return obj;
}


const pman_page_t page_auto = {
    .create        = create_page,
    .destroy       = destroy_page,
    .open          = open_page,
    .close         = close_page,
    .process_event = page_event,
};