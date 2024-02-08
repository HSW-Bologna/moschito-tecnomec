#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "peripherals/buzzer.h"
#include "peripherals/digout.h"
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/rtc/RX8010/rx8010.h"
#include "I2C/i2c_devices/io/MCP23008/mcp23008.h"
#include "watcher.h" /* /components/c-watcher */
#include "esp_log.h"
#include "utils/utils.h"
#include "erogator/erogator.h"
#include "configuration.h"
#include "observer.h"
#include "wifi/network.h"
#include "wifi/server.h"
#include "rtc/rtc.h"
#include "rtc/model_watcher.h"


void erogators_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg);


static const char *TAG = "Controller";


/* watcher instance */
static watcher_t watcher;


void controller_init(model_t *pmodel) {
    ESP_LOGI(TAG, "TEST SIZE 1: %zu", sizeof(time_t));

    erogator_init();

    // Rome, no DST
    setenv("TZ", "UTC-2", 1);
    tzset();

    if (rx8010_is_stopped(rtc_driver)) {
        ESP_LOGI(TAG, "RTC was stopped, initializing...");
        rtc_time_t rtc_time = {.day = 6, .wday = 1, .month = 3, .year = 22};
        rx8010_set_stop(rtc_driver, 0);
        rx8010_set_time(rtc_driver, rtc_time);
        ESP_LOGI(TAG, "RTC Clock started");
    } else {
        rtc_time_t rtc_time = {0};
        rx8010_get_time(rtc_driver, &rtc_time);
        struct tm tm = rx8010_tm_from_rtc(rtc_time);
        tm.tm_isdst  = -1;
        utils_set_system_time(&tm);
    }


    buzzer_beep(2, 50, 100);

    watcher_init(&watcher, pmodel);
    WATCHER_ADD_ENTRY(&watcher, &pmodel->run.erogators_state, (void *) erogators_cb, NULL);

    configuration_load(pmodel);
    observer_init(pmodel);
    erogator_refresh(pmodel);

    view_change_page(pmodel, &page_main);

    wifi_init_softap(pmodel);
    rtc_initt(pmodel);

    ESP_LOGI(TAG, "Controller initialized");
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_EROGATION:
            ESP_LOGI(TAG, "Toggling (on) erogator %i for %is", msg->erogator, model_get_erogation_seconds(pmodel));
            if (model_get_erogators_state(pmodel) == EROGATORS_STATE_OFF) {
                erogator_run(pmodel, msg->erogator, model_get_erogation_seconds(pmodel));
            } else {
                erogator_stop(pmodel);
            }
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_START_EROGATION:
            erogator_run(pmodel, msg->erogator, model_get_erogation_seconds(pmodel));
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_STOP_EROGATION:
            erogator_stop(pmodel);
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SAVE_RTC_TIME: {
            utils_set_system_time(&msg->time_info);
            break;
        }
    }
}


void controller_manage(model_t *pmodel) {
    watcher_watch(&watcher, get_millis());

    observer_observe(pmodel);
    erogator_manage(pmodel);

    model_watcher_watch();

    rtc_ws_msg_t rtc_ws_msg;
    while (rtc_ws_get_next_msg(&rtc_ws_msg)) {
        rtc_message_recvd_handler(rtc_ws_msg.data, rtc_ws_msg.size, pmodel);
        free(rtc_ws_msg.data);
    }
}


void erogators_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg) {
    (void) old_buffer;
    (void) memory;
    (void) size;
    (void) arg;

    model_t *pmodel = (model_t *) user_ptr;
    switch (model_get_erogators_state(pmodel)) {
        case EROGATORS_STATE_OFF:
            DIGOUT_CLEAR(DIGOUT_PUMP);
            DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
            DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
            break;

        case EROGATORS_STATE_1:
            DIGOUT_SET(DIGOUT_PUMP);
            DIGOUT_SET(DIGOUT_EROGATOR_1);
            DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
            break;

        case EROGATORS_STATE_2:
            DIGOUT_SET(DIGOUT_PUMP);
            DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
            DIGOUT_SET(DIGOUT_EROGATOR_2);
            break;
    }
}
