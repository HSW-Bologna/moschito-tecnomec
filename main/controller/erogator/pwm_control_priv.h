#ifndef PWM_CONTROL_PRIV_H_INCLUDED
#define PWM_CONTROL_PRIV_H_INCLUDED


#include "model/model.h"


void erogator_pwm_control_init(void);
void erogator_pwm_control_on(model_t *pmodel, erogator_t erogator);
void erogator_pwm_control_off(void);


#endif