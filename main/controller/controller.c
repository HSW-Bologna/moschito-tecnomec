#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "peripherals/buzzer.h"
#include "peripherals/digout.h"
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/io/MCP23008/mcp23008.h"
#include "gel/data_structures/watcher.h"
#include "esp_log.h"
#include "utils/utils.h"
#include "erogator.h"


#define NUM_WATCHED_VARIABLES 1


void erogators_cb(void *mem, void *args);


static const char *TAG = "Controller";


watcher_t watched_variables[NUM_WATCHED_VARIABLES + 1];


void controller_init(model_t *pmodel) {
    erogator_init();

    // Rome, no DST
    setenv("TZ", "UTC-2", 1);
    tzset();

    buzzer_beep(2, 50, 100);

    size_t i               = 0;
    watched_variables[i++] = WATCHER(&pmodel->run.erogators_state, erogators_cb, pmodel);
    assert(i == NUM_WATCHED_VARIABLES);
    watched_variables[i] = WATCHER_NULL;

    watcher_list_init(watched_variables);

    view_change_page(pmodel, &page_main);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_EROGATION:
            if (erogator_get_state() == EROGATORS_STATE_OFF) {
                erogator_run(msg->erogator, model_get_erogation_seconds(pmodel));
            } else {
                erogator_stop();
            }
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_START_EROGATION:
            erogator_run(msg->erogator, model_get_erogation_seconds(pmodel));
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_STOP_EROGATION:
            erogator_stop();
            break;
    }
}


void controller_manage(model_t *pmodel) {
    (void)pmodel;
    watcher_process_changes(watched_variables, get_millis());

    if (model_set_erogators_state(pmodel, erogator_get_state())) {
        view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
    }
}


void erogators_cb(void *mem, void *args) {
    model_t *pmodel = args;
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