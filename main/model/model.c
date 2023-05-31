#include <stdlib.h>
#include <assert.h>
#include "model.h"
#include "model_descriptor.h"
#include "gel/scheduler/scheduler.h"
#include "config/app_config.h"


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);

    model_descriptor_init();

    pmodel->configuration.language                         = 0;
    pmodel->configuration.erogation_seconds                = 5;
    pmodel->configuration.brightness                       = 100;
    pmodel->configuration.volume                           = 1;
    pmodel->configuration.erogator_percentages[EROGATOR_1] = 100;
    pmodel->configuration.erogator_percentages[EROGATOR_2] = 100;

    pmodel->run.stop                        = 0;
    pmodel->run.stop_time                   = 0;
    pmodel->run.erogators_state             = EROGATORS_STATE_OFF;
    pmodel->run.missing_water_alarm         = 0;
    pmodel->run.missing_product[EROGATOR_1] = 0;
    pmodel->run.missing_product[EROGATOR_2] = 0;

    for (erogator_t erogator = EROGATOR_1; erogator <= EROGATOR_2; erogator++) {
        for (size_t i = 0; i < NUM_PROGRAMS; i++) {
            pmodel->configuration.active_seconds[erogator][i] = 15;
            pmodel->configuration.pause_seconds[erogator][i]  = 15 * 60;
            pmodel->configuration.working_modes[erogator][i]  = WORKING_MODE_CONTINUOUS;
        }
    }

    scheduler_init(&pmodel->configuration.schedulers[EROGATOR_1]);
    scheduler_init(&pmodel->configuration.schedulers[EROGATOR_2]);
}


uint8_t model_set_missing_product_alarm(model_t *pmodel, erogator_t erogator, uint8_t value) {
    assert(pmodel != NULL);
    value = value > 0;
    if (pmodel->run.missing_product[erogator] != value) {
        pmodel->run.missing_product[erogator] = value;
        return 1;
    } else {
        return 0;
    }
}


uint8_t model_get_missing_product(model_t *pmodel, erogator_t erogator) {
    assert(pmodel != NULL);
    return pmodel->run.missing_product[erogator];
}


void model_start_erogator(model_t *pmodel, erogator_t erogator) {
    assert(pmodel != NULL);
    switch (erogator) {
        case EROGATOR_1:
            pmodel->run.erogators_state = EROGATORS_STATE_1;
            break;

        case EROGATOR_2:
            pmodel->run.erogators_state = EROGATORS_STATE_2;
            break;
    }
}


void model_toggle_erogator(model_t *pmodel, erogator_t erogator) {
    assert(pmodel != NULL);
    if (model_get_erogators_state(pmodel) == EROGATORS_STATE_OFF) {
        switch (erogator) {
            case EROGATOR_1:
                pmodel->run.erogators_state = EROGATORS_STATE_1;
                break;

            case EROGATOR_2:
                pmodel->run.erogators_state = EROGATORS_STATE_2;
                break;
        }
    } else {
        pmodel->run.erogators_state = EROGATORS_STATE_OFF;
    }
}


void model_stop_erogator(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.erogators_state = EROGATORS_STATE_OFF;
}


uint8_t model_get_erogator_percentage(model_t *pmodel, erogator_t erogator) {
    assert(pmodel != NULL);
    return pmodel->configuration.erogator_percentages[erogator];
}


void model_set_erogator_percentage(model_t *pmodel, erogator_t erogator, uint8_t percentage) {
    assert(pmodel != NULL);
    if (percentage > 100) {
        percentage = 100;
    }
    pmodel->configuration.erogator_percentages[erogator] = percentage;
}


uint8_t model_is_program_enabled(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    return scheduler_is_schedule_entry_enabled(&pmodel->configuration.schedulers[erogator], program);
}


void model_check_program_for_consistency(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL);

    erogator_t         other_erogator = erogator == EROGATOR_1 ? EROGATOR_2 : EROGATOR_1;
    scheduler_entry_t *entry          = scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program);

    if (entry->enabled) {
        if (scheduler_are_there_overlapping_entries(&pmodel->configuration.schedulers[other_erogator], entry)) {
            entry->enabled = 0;
        }
    }
}


int model_toggle_program(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    scheduler_entry_t *entry       = scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program);
    uint8_t            was_enabled = entry->enabled;

    if (!was_enabled) {
        erogator_t other_erogator = erogator == EROGATOR_1 ? EROGATOR_2 : EROGATOR_1;
        // Check for conflicts
        if (scheduler_are_there_overlapping_entries(&pmodel->configuration.schedulers[other_erogator], entry)) {
            return -1;
        }
    }

    entry->enabled = !was_enabled;
    return 0;
}


void model_clear_all_programs(model_t *pmodel, erogator_t erogator) {
    assert(pmodel != NULL);
    for (size_t i = 0; i < NUM_PROGRAMS; i++) {
        scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], i)->enabled = 0;
    }
}


uint8_t model_get_program_days(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    return scheduler_get_entry(&pmodel->configuration.schedulers[erogator], program)->days;
}


void model_set_program_days(model_t *pmodel, erogator_t erogator, size_t program, uint8_t days) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program)->days = days;
    model_check_program_for_consistency(pmodel, erogator, program);
}


unsigned long model_get_program_start_second(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    return scheduler_get_entry(&pmodel->configuration.schedulers[erogator], program)->start_second;
}


void model_set_program_start_second(model_t *pmodel, erogator_t erogator, size_t program, unsigned long start_second) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    scheduler_entry_t *entry = scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program);

    unsigned long duration = scheduler_get_entry_duration(entry);
    entry->start_second    = start_second;

    if (entry->start_second > entry->end_second) {
        entry->end_second = entry->start_second + duration;
        if (entry->end_second > 3600 * 24) {
            entry->end_second = 3600 * 24 - 1;
        }
    } else if (scheduler_get_entry_duration(entry) > APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS) {
        entry->end_second = entry->start_second + duration;
    }

    model_check_program_for_consistency(pmodel, erogator, program);
}


unsigned long model_get_program_stop_second(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    return scheduler_get_entry(&pmodel->configuration.schedulers[erogator], program)->end_second;
}


void model_set_program_stop_second(model_t *pmodel, erogator_t erogator, size_t program, unsigned long stop_second) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program)->end_second = stop_second;
    model_check_program_for_consistency(pmodel, erogator, program);
}


void model_toggle_program_day(model_t *pmodel, erogator_t erogator, size_t program, scheduler_dow_t day) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    scheduler_entry_t *entry = scheduler_get_entry_mut(&pmodel->configuration.schedulers[erogator], program);
    if ((entry->days & (1 << day)) > 0) {
        entry->days &= ~(1 << day);
    } else {
        entry->days |= 1 << day;
    }
}


working_mode_t model_get_working_mode(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    return pmodel->configuration.working_modes[erogator][program];
}


void model_toggle_working_mode(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    pmodel->configuration.working_modes[erogator][program] = !pmodel->configuration.working_modes[erogator][program];

    if (pmodel->configuration.working_modes[erogator][program] == WORKING_MODE_CONTINUOUS) {
        if (model_get_program_start_second(pmodel, erogator, program) + APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS <
            model_get_program_stop_second(pmodel, erogator, program)) {
            model_set_program_stop_second(pmodel, erogator, program,
                                          model_get_program_start_second(pmodel, erogator, program) +
                                              APP_CONFIG_MAX_CONTINUOUS_DURATION_SECONDS);
        }
    }
}


unsigned long model_get_erogation_active_time(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    return pmodel->configuration.active_seconds[erogator][program];
}


void model_set_erogation_active_time(model_t *pmodel, erogator_t erogator, size_t program, unsigned long seconds) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    pmodel->configuration.active_seconds[erogator][program] = seconds;
}


unsigned long model_get_erogation_pause_time(model_t *pmodel, erogator_t erogator, size_t program) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);

    return pmodel->configuration.pause_seconds[erogator][program];
}


void model_set_erogation_pause_time(model_t *pmodel, erogator_t erogator, size_t program, unsigned long seconds) {
    assert(pmodel != NULL && program < NUM_PROGRAMS);
    pmodel->configuration.pause_seconds[erogator][program] = seconds;
}


int model_get_scheduler_active_erogator(model_t *pmodel, erogator_t *erogator) {
    assert(pmodel != NULL);

    time_t    now       = time(NULL);
    struct tm time_info = *localtime(&now);

    int found = scheduler_get_active_entry_number(&pmodel->configuration.schedulers[EROGATOR_1], &time_info);
    if (found >= 0) {
        *erogator = EROGATOR_1;
    } else {
        found = scheduler_get_active_entry_number(&pmodel->configuration.schedulers[EROGATOR_2], &time_info);
        if (found >= 0) {
            *erogator = EROGATOR_2;
        }
    }

    return found;
}


void model_toggle_stop(model_t *pmodel) {
    assert(pmodel != NULL);

    pmodel->run.stop      = !model_is_erogation_stopped(pmodel);
    pmodel->run.stop_time = time(NULL);
}


uint8_t model_is_erogation_stopped(model_t *pmodel) {
    assert(pmodel != NULL);
    if (pmodel->run.stop == 0) {
        return 0;
    } else {
        time_t now = time(NULL);
        return (now >= pmodel->run.stop_time) && (now - pmodel->run.stop_time < (time_t)(24UL * 60UL * 60UL));
    }
}
