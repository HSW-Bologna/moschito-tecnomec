#include <stdlib.h>
#include <assert.h>
#include "model.h"

void model_init(model_t *pmodel) {
    assert(pmodel != NULL);

    pmodel->configuration.language          = 0;
    pmodel->configuration.erogation_seconds = 5;

    pmodel->run.stop            = 0;
    pmodel->run.erogators_state = EROGATORS_STATE_OFF;
}


uint16_t model_get_language(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->configuration.language;
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