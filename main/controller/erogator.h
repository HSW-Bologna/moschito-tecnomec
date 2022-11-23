#ifndef EROGATOR_H_INCLUDED
#define EROGATOR_H_INCLUDED


#include <stdint.h>
#include "model/model.h"


void              erogator_init(void);
void              erogator_run(erogator_t erogator, uint16_t seconds);
void              erogator_stop(void);
erogators_state_t erogator_get_state(void);


#endif