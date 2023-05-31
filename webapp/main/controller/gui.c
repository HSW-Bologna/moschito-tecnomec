#include "model/model.h"
#include "view/view.h"
#include "controller.h"
#include "esp_log.h"
#include "lvgl.h"


static const char *TAG = "Gui";


void controller_gui_manage(model_t *pmodel) {
    (void) TAG;

    /* todo: hardcoded */
    lv_tick_inc(10);
    lv_timer_handler();

    view_message_t umsg;
    view_event_t event;

    while (view_get_next_msg(pmodel, &umsg, &event)) {
        controller_process_message(pmodel, &umsg.cmsg);
        view_process_msg(umsg.vmsg, pmodel);
    }
}
