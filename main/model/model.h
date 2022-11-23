#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stddef.h>


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


typedef struct {
    struct {
        uint16_t language;
        uint16_t erogation_seconds;
    } configuration;

    struct {
        uint8_t           stop;
        erogators_state_t erogators_state;
    } run;
} model_t;


void     model_init(model_t *pmodel);
uint16_t model_get_language(model_t *pmodel);
void     model_start_erogator(model_t *pmodel, erogator_t erogator);
void     model_stop_erogator(model_t *pmodel);
void     model_toggle_erogator(model_t *pmodel, erogator_t erogator);


GETTERNSETTER(stop, run.stop);

GETTERNSETTER(erogators_state, run.erogators_state);
GETTERNSETTER(erogation_seconds, configuration.erogation_seconds);

TOGGLER(stop, run.stop);


#endif