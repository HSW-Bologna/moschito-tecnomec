#include "controller.h"
#include "model/model.h"
#include "rtc/rtc.h"
#include "rtc/model_watcher.h"
#include "view/view.h"
#include "esp_log.h"


static const char *TAG = "Controller";


void controller_init(model_t *pmodel) {
    view_change_page(pmodel, &page_main);
    rtc_init(pmodel);

    ESP_LOGI(TAG, "Controller initialized");
}

void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    (void) pmodel;
    (void) msg;

    rtc_send_controller_message(msg);
}

void controller_manage(model_t *pmodel) {
    (void) pmodel;

    model_watcher_watch();
}
