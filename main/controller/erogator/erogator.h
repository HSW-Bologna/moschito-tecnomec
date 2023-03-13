#ifndef EROGATOR_H_INCLUDED
#define EROGATOR_H_INCLUDED


#include <stdint.h>
#include "model/model.h"


void erogator_init(void);
void erogator_run(model_t *pmodel, erogator_t erogator, uint16_t seconds);
void erogator_stop(model_t *pmodel);
void erogator_manage(model_t *pmodel);
void erogator_refresh(model_t *pmodel);


#endif