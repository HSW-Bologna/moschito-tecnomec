#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "gel/state_machine/state_machine.h"
#include "gel/timer/timer.h"
#include "peripherals/buzzer.h"
#include "erogator.h"
#include "model/model.h"
#include "utils/utils.h"
#include "view/view.h"
#include "pwm_control_priv.h"


#define START_DELAY 2000UL


typedef enum {
    EROGATOR_SM_STATE_OFF = 0,
    EROGATOR_SM_STATE_ON_MANUAL_WAITING,
    EROGATOR_SM_STATE_ON_MANUAL,
    EROGATOR_SM_STATE_ON_AUTO,
    EROGATOR_SM_STATE_OFF_AUTO,
} erogator_sm_state_t;


typedef enum {
    EROGATOR_EVENT_TAG_START_MANUAL = 0,
    EROGATOR_EVENT_TAG_START_DELAY_DONE,
    EROGATOR_EVENT_TAG_STOP,
    EROGATOR_EVENT_TAG_REFRESH,
    EROGATOR_EVENT_TAG_BLINK,
} erogator_event_tag_t;


typedef struct {
    erogator_event_tag_t tag;

    union {
        struct {
            uint16_t   seconds;
            erogator_t erogator;
        };
    };
} erogator_event_t;


DEFINE_STATE_MACHINE(erogator, erogator_event_t, model_t);


static void    gel_timer_callback(gel_timer_t *timer, void *pmodel, void *arg);
static void    start_erogator(model_t *pmodel, erogator_t erogator);
static void    stop_everything(model_t *pmodel);
static uint8_t schedule_active(model_t *pmodel);

static int off_event_manager(model_t *pmodel, erogator_event_t event);
static int on_manual_waiting_event_manager(model_t *pmodel, erogator_event_t event);
static int on_manual_event_manager(model_t *pmodel, erogator_event_t event);
static int on_auto_event_manager(model_t *pmodel, erogator_event_t event);
static int off_auto_event_manager(model_t *pmodel, erogator_event_t event);


static gel_timer_t              delay_timer  = {0};
static gel_timer_t              manual_timer = {0};
static gel_timer_t              blink_timer  = {0};
static erogator_event_manager_t managers[]   = {
    [EROGATOR_SM_STATE_OFF]               = off_event_manager,
    [EROGATOR_SM_STATE_ON_MANUAL_WAITING] = on_manual_waiting_event_manager,
    [EROGATOR_SM_STATE_ON_MANUAL]         = on_manual_event_manager,
    [EROGATOR_SM_STATE_ON_AUTO]           = on_auto_event_manager,
    [EROGATOR_SM_STATE_OFF_AUTO]          = off_auto_event_manager,
};
static erogator_state_machine_t erogator_sm = {
    .state    = EROGATOR_SM_STATE_OFF,
    .managers = managers,
};
static erogator_t  current_erogator = 0;
static const char *TAG              = "Erogator";


void erogator_init(void) {
    erogator_pwm_control_init();
}


void erogator_manage(model_t *pmodel) {
    gel_timer_manage_callbacks(&manual_timer, 1, get_millis(), pmodel);
    gel_timer_manage_callbacks(&delay_timer, 1, get_millis(), pmodel);
    gel_timer_manage_callbacks(&blink_timer, 1, get_millis(), pmodel);
}


void erogator_run(model_t *pmodel, erogator_t erogator, uint16_t seconds) {
    erogator_sm_send_event(&erogator_sm, pmodel,
                           (erogator_event_t){
                               .tag      = EROGATOR_EVENT_TAG_START_MANUAL,
                               .seconds  = seconds,
                               .erogator = erogator,
                           });
}


void erogator_refresh(model_t *pmodel) {
    erogator_sm_send_event(&erogator_sm, pmodel, (erogator_event_t){.tag = EROGATOR_EVENT_TAG_REFRESH});
}


void erogator_stop(model_t *pmodel) {
    erogator_sm_send_event(&erogator_sm, pmodel, (erogator_event_t){.tag = EROGATOR_EVENT_TAG_STOP});
}


static int off_event_manager(model_t *pmodel, erogator_event_t event) {
    switch (event.tag) {
        case EROGATOR_EVENT_TAG_START_MANUAL: {
            buzzer_beep(1, 1000, 0);

            unsigned long now = get_millis();

            gel_timer_activate(&delay_timer, START_DELAY, now, gel_timer_callback,
                               (void *)(uintptr_t)EROGATOR_EVENT_TAG_START_DELAY_DONE);
            gel_timer_activate(&manual_timer, event.seconds * 1000UL, now, gel_timer_callback,
                               (void *)(uintptr_t)EROGATOR_EVENT_TAG_STOP);
            gel_timer_pause(&manual_timer, now);

            current_erogator = event.erogator;

            return EROGATOR_SM_STATE_ON_MANUAL_WAITING;
        }

        case EROGATOR_EVENT_TAG_REFRESH:
            if (schedule_active(pmodel)) {
                return EROGATOR_SM_STATE_ON_AUTO;
            } else {
                return -1;
            }

        default:
            return -1;
    }
}


static int on_manual_waiting_event_manager(model_t *pmodel, erogator_event_t event) {
    switch (event.tag) {
        case EROGATOR_EVENT_TAG_START_DELAY_DONE:
            start_erogator(pmodel, current_erogator);
            gel_timer_resume(&manual_timer, get_millis());
            return EROGATOR_SM_STATE_ON_MANUAL;

        case EROGATOR_EVENT_TAG_STOP:
            gel_timer_deactivate(&manual_timer);
            gel_timer_deactivate(&delay_timer);
            stop_everything(pmodel);
            if (schedule_active(pmodel)) {
                return EROGATOR_SM_STATE_ON_AUTO;
            } else {
                return EROGATOR_SM_STATE_OFF;
            }

        case EROGATOR_EVENT_TAG_REFRESH:
            if (model_is_erogation_stopped(pmodel)) {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            } else {
                return -1;
            }

        default:
            return -1;
    }
}


static int on_manual_event_manager(model_t *pmodel, erogator_event_t event) {
    switch (event.tag) {
        case EROGATOR_EVENT_TAG_STOP:
            gel_timer_deactivate(&manual_timer);
            gel_timer_deactivate(&delay_timer);
            stop_everything(pmodel);
            return EROGATOR_SM_STATE_OFF;

        case EROGATOR_EVENT_TAG_REFRESH:
            if (model_is_erogation_stopped(pmodel)) {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            } else {
                return -1;
            }

        default:
            return -1;
    }
}


static int on_auto_event_manager(model_t *pmodel, erogator_event_t event) {
    switch (event.tag) {
        case EROGATOR_EVENT_TAG_REFRESH:
            if (model_is_erogation_stopped(pmodel)) {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            } else if (schedule_active(pmodel)) {
                return -1;
            } else {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            }

        case EROGATOR_EVENT_TAG_BLINK: {
            erogator_t erogator;
            int        program = model_get_scheduler_active_erogator(pmodel, &erogator);

            if (program >= 0) {
                if (model_get_working_mode(pmodel, erogator, program) == WORKING_MODE_TIMED) {
                    erogator_pwm_control_off();

                    gel_timer_activate(&blink_timer,
                                       model_get_erogation_pause_time(pmodel, current_erogator, program) * 1000UL,
                                       get_millis(), gel_timer_callback, (void *)(uintptr_t)EROGATOR_EVENT_TAG_BLINK);

                    return EROGATOR_SM_STATE_OFF_AUTO;
                } else {
                    return -1;
                }
            } else {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            }
        }

        default:
            return -1;
    }
}


static int off_auto_event_manager(model_t *pmodel, erogator_event_t event) {
    switch (event.tag) {
        case EROGATOR_EVENT_TAG_REFRESH:
            if (model_is_erogation_stopped(pmodel)) {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            } else if (schedule_active(pmodel)) {
                return -1;
            } else {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            }

        case EROGATOR_EVENT_TAG_BLINK: {
            erogator_t erogator;
            int        program = model_get_scheduler_active_erogator(pmodel, &erogator);

            if (program >= 0) {
                erogator_pwm_control_on(pmodel, erogator);

                gel_timer_activate(&blink_timer,
                                   model_get_erogation_active_time(pmodel, current_erogator, program) * 1000UL,
                                   get_millis(), gel_timer_callback, (void *)(uintptr_t)EROGATOR_EVENT_TAG_BLINK);

                return EROGATOR_SM_STATE_ON_AUTO;
            } else {
                stop_everything(pmodel);
                return EROGATOR_SM_STATE_OFF;
            }
        }

        default:
            return -1;
    }
}


static void gel_timer_callback(gel_timer_t *timer, void *pmodel, void *arg) {
    erogator_sm_send_event(&erogator_sm, pmodel, (erogator_event_t){.tag = (erogator_event_tag_t)(uintptr_t)arg});
}


static void stop_everything(model_t *pmodel) {
    erogator_pwm_control_off();
    model_set_erogators_state(pmodel, EROGATORS_STATE_OFF);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static void start_erogator(model_t *pmodel, erogator_t erogator) {
    erogator_pwm_control_on(pmodel, erogator);
    switch (erogator) {
        case EROGATOR_1:
            model_set_erogators_state(pmodel, EROGATORS_STATE_1);
            break;
        case EROGATOR_2:
            model_set_erogators_state(pmodel, EROGATORS_STATE_1);
            break;
    }

    view_event((view_event_t){.code = VIEW_EVENT_CODE_UPDATE});
}


static uint8_t schedule_active(model_t *pmodel) {
    erogator_t erogator;

    int program = model_get_scheduler_active_erogator(pmodel, &erogator);

    if (program >= 0) {
        gel_timer_activate(&blink_timer, model_get_erogation_active_time(pmodel, current_erogator, program) * 1000UL,
                           get_millis(), gel_timer_callback, (void *)(uintptr_t)EROGATOR_EVENT_TAG_BLINK);

        start_erogator(pmodel, erogator);
        return 1;
    } else {
        gel_timer_deactivate(&blink_timer);
        stop_everything(pmodel);
        return 0;
    }
}
