#include <assert.h>
#include "pwm_control_priv.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timecheck.h"
#include "peripherals/digout.h"
#include "erogator.h"
#include "model/model.h"
#include "utils/utils.h"
#include "view/view.h"



#define PWM_PERIOD 5000UL


static void timer_cb(TimerHandle_t timer);


static SemaphoreHandle_t sem              = NULL;
static TimerHandle_t     timer            = NULL;
static erogator_t        current_erogator = 0;
static uint32_t          on_period        = 0;
static uint32_t          off_period       = 0;
static unsigned long     timestamp        = 0;
static uint8_t           pwm_on           = 0;
static uint8_t           output_on        = 0;
static const char       *TAG              = "Erogator PWM";


void erogator_pwm_control_init(void) {
    assert(sem == NULL);
    static StaticSemaphore_t semaphore_buffer;
    sem = xSemaphoreCreateMutexStatic(&semaphore_buffer);

    assert(timer == NULL);
    static StaticTimer_t timer_buffer;
    timer = xTimerCreateStatic(TAG, pdMS_TO_TICKS(100), 1, NULL, timer_cb, &timer_buffer);
    xTimerStart(timer, portMAX_DELAY);
}


void erogator_pwm_control_on(model_t *pmodel, erogator_t erogator) {
    xSemaphoreTake(sem, portMAX_DELAY);
    current_erogator = erogator;
    on_period        = (PWM_PERIOD * model_get_erogator_percentage(pmodel, erogator)) / 100;
    off_period       = PWM_PERIOD - on_period;
    if (pwm_on == 0) {
        output_on = 1;
        pwm_on    = 1;

        switch (erogator) {
            case EROGATOR_1:
                DIGOUT_SET(DIGOUT_EROGATOR_1);
                break;
            case EROGATOR_2:
                DIGOUT_SET(DIGOUT_EROGATOR_2);
                break;
        }

        timestamp = get_millis();
    }
    xSemaphoreGive(sem);
    DIGOUT_CLEAR(DIGOUT_PUMP);
}


void erogator_pwm_control_off(void) {
    DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
    DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
    DIGOUT_CLEAR(DIGOUT_PUMP);
    xSemaphoreTake(sem, portMAX_DELAY);
    pwm_on = 0;
    xSemaphoreGive(sem);
}


static void timer_cb(TimerHandle_t timer) {
    xSemaphoreTake(sem, portMAX_DELAY);
    if (pwm_on && is_expired(timestamp, get_millis(), output_on ? on_period : off_period)) {
        if (output_on) {
            DIGOUT_CLEAR(DIGOUT_EROGATOR_1);
            DIGOUT_CLEAR(DIGOUT_EROGATOR_2);
        } else {
            switch (current_erogator) {
                case EROGATOR_1:
                    DIGOUT_SET(DIGOUT_EROGATOR_1);
                    break;
                case EROGATOR_2:
                    DIGOUT_SET(DIGOUT_EROGATOR_2);
                    break;
            }
        }

        output_on = !output_on;
        timestamp = get_millis();
    }
    xSemaphoreGive(sem);
}