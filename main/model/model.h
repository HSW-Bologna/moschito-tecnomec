#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include "gel/scheduler/scheduler.h"


#define LANGUAGE_ITALIANO 0
#define LANGUAGE_ENGLISH  1
#define NUM_EROGATORS     2
#define NUM_PROGRAMS      GEL_SCHEDULER_MAX_ENTRIES


#define GETTER(name, field)                                                                                            \
    static inline                                                                                                      \
        __attribute__((always_inline, const)) typeof(((model_t *)0)->field) model_get_##name(model_t *pmodel) {        \
        assert(pmodel != NULL);                                                                                        \
        return pmodel->field;                                                                                          \
    }

#define SETTER(name, field)                                                                                            \
    static inline __attribute__((always_inline))                                                                       \
    uint8_t model_set_##name(model_t *pmodel, typeof(((model_t *)0)->field) value) {                                   \
        assert(pmodel != NULL);                                                                                        \
        if (pmodel->field != value) {                                                                                  \
            pmodel->field = value;                                                                                     \
            return 1;                                                                                                  \
        } else {                                                                                                       \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#define TOGGLER(name, field)                                                                                           \
    static inline __attribute__((always_inline)) void model_toggle_##name(model_t *pmodel) {                           \
        assert(pmodel != NULL);                                                                                        \
        pmodel->field = !pmodel->field;                                                                                \
    }

#define GETTERNSETTER(name, field)                                                                                     \
    GETTER(name, field)                                                                                                \
    SETTER(name, field)


typedef enum {
    EROGATOR_1 = 0,
    EROGATOR_2,
} erogator_t;


typedef enum {
    EROGATORS_STATE_OFF = 0,
    EROGATORS_STATE_1,
    EROGATORS_STATE_2,
} erogators_state_t;


typedef enum {
    WORKING_MODE_CONTINUOUS,
    WORKING_MODE_TIMED,
} working_mode_t;


typedef struct {
    struct {
        uint16_t language;
        uint16_t erogation_seconds;
        uint8_t  brightness;
        uint8_t  volume;
        uint8_t  erogator_percentages[NUM_EROGATORS];
        uint8_t  working_modes[NUM_EROGATORS][NUM_PROGRAMS];
        uint32_t active_seconds[NUM_EROGATORS][NUM_PROGRAMS];
        uint32_t pause_seconds[NUM_EROGATORS][NUM_PROGRAMS];

        scheduler_t schedulers[NUM_EROGATORS];
    } configuration;

    struct {
        uint8_t           stop;
        erogators_state_t erogators_state;
    } run;
} model_t;


void          model_init(model_t *pmodel);
void          model_start_erogator(model_t *pmodel, erogator_t erogator);
void          model_stop_erogator(model_t *pmodel);
void          model_toggle_erogator(model_t *pmodel, erogator_t erogator);
uint8_t       model_get_erogator_percentage(model_t *pmodel, erogator_t erogator);
void          model_set_erogator_percentage(model_t *pmodel, erogator_t erogator, uint8_t percentage);
uint8_t       model_is_program_enabled(model_t *pmodel, erogator_t erogator, size_t program);
int           model_toggle_program(model_t *pmodel, erogator_t erogator, size_t program);
void          model_clear_all_programs(model_t *pmodel, erogator_t erogator);
uint8_t       model_get_program_days(model_t *pmodel, erogator_t erogator, size_t program);
void          model_set_program_days(model_t *pmodel, erogator_t erogator, size_t program, uint8_t days);
unsigned long model_get_program_start_second(model_t *pmodel, erogator_t erogator, size_t program);
unsigned long model_get_program_stop_second(model_t *pmodel, erogator_t erogator, size_t program);
void model_set_program_start_second(model_t *pmodel, erogator_t erogator, size_t program, unsigned long start_second);
void model_set_program_stop_second(model_t *pmodel, erogator_t erogator, size_t program, unsigned long stop_second);
working_mode_t model_get_working_mode(model_t *pmodel, erogator_t erogator, size_t program);
void           model_toggle_working_mode(model_t *pmodel, erogator_t erogator, size_t program);
unsigned long  model_get_erogation_active_time(model_t *pmodel, erogator_t erogator, size_t program);
unsigned long  model_get_erogation_pause_time(model_t *pmodel, erogator_t erogator, size_t program);
void model_set_erogation_active_time(model_t *pmodel, erogator_t erogator, size_t program, unsigned long seconds);
void model_set_erogation_pause_time(model_t *pmodel, erogator_t erogator, size_t program, unsigned long seconds);
void model_check_program_for_consistency(model_t *pmodel, erogator_t erogator, size_t program);


GETTERNSETTER(stop, run.stop);

GETTERNSETTER(language, configuration.language);
GETTERNSETTER(erogators_state, run.erogators_state);
GETTERNSETTER(erogation_seconds, configuration.erogation_seconds);
GETTERNSETTER(volume, configuration.volume);
GETTERNSETTER(brightness, configuration.brightness);

TOGGLER(stop, run.stop);


#endif