#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "peripherals/digout.h"
#include "peripherals/buzzer.h"
#include "erogator.h"
#include "model/model.h"


#define EROGATOR_1_BIT 0x01
#define EROGATOR_2_BIT 0x02


static void stop_timer_cb(TimerHandle_t timer);
static void start_timer_cb(TimerHandle_t timer);


static const char        *TAG         = "Erogation";
static TimerHandle_t      stop_timer  = NULL;
static TimerHandle_t      start_timer = NULL;
static EventGroupHandle_t event_group = NULL;


void erogator_init(void) {
    assert(start_timer == NULL && stop_timer == NULL && event_group == NULL);
    static StaticTimer_t stop_timer_buffer;
    stop_timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(100), 0, NULL, stop_timer_cb, &stop_timer_buffer);

    static StaticTimer_t start_timer_buffer;
    start_timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(2000), 0, NULL, start_timer_cb, &start_timer_buffer);

    static StaticEventGroup_t event_group_buffer;
    event_group = xEventGroupCreateStatic(&event_group_buffer);
}


void erogator_run(erogator_t erogator, uint16_t seconds) {
    buzzer_beep(1, 1000, 0);
    xTimerChangePeriod(stop_timer, pdMS_TO_TICKS(seconds * 1000UL), portMAX_DELAY);
    vTimerSetTimerID(start_timer, (void *)(uintptr_t)erogator);
    xTimerStart(start_timer, portMAX_DELAY);
}


void erogator_stop(void) {
    xTimerStop(start_timer, portMAX_DELAY);
    xTimerStop(stop_timer, portMAX_DELAY);
    DIGOUT_CLEAR(DIGOUT_PUMP);
    DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
    DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
    xEventGroupClearBits(event_group, EROGATOR_1_BIT | EROGATOR_2_BIT);
}


erogators_state_t erogator_get_state(void) {
    uint32_t bits = xEventGroupGetBits(event_group);
    if ((bits & EROGATOR_1_BIT) > 0) {
        return EROGATORS_STATE_1;
    } else if ((bits & EROGATOR_2_BIT) > 0) {
        return EROGATORS_STATE_2;
    } else {
        return EROGATORS_STATE_OFF;
    }
}


static void stop_timer_cb(TimerHandle_t timer) {
    DIGOUT_CLEAR(DIGOUT_PUMP);
    DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
    DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
    xEventGroupClearBits(event_group, EROGATOR_1_BIT | EROGATOR_2_BIT);
}


static void start_timer_cb(TimerHandle_t timer) {
    erogator_t erogator = (erogator_t)(uintptr_t)pvTimerGetTimerID(timer);
    switch (erogator) {
        case EROGATOR_1:
            DIGOUT_SET(DIGOUT_PUMP);
            DIGOUT_SET(DIGOUT_EROGATOR_1);
            xEventGroupSetBits(event_group, EROGATOR_1_BIT);
            break;

        case EROGATOR_2:
            DIGOUT_SET(DIGOUT_PUMP);
            DIGOUT_SET(DIGOUT_EROGATOR_2);
            xEventGroupSetBits(event_group, EROGATOR_2_BIT);
            break;
    }
    xTimerStart(stop_timer, portMAX_DELAY);
}