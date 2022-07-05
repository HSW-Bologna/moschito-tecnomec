#ifndef HARDWAREPROFILE_H_INCLUDED
#define HARDWAREPROFILE_H_INCLUDED

#include <driver/gpio.h>

/*
 * Definizioni dei pin da utilizzare
 */


#define HAP_BUZZER GPIO_NUM_4

#define HAP_RX1 GPIO_NUM_27
#define HAP_TX1 GPIO_NUM_5

#define HAP_ON_OFF GPIO_NUM_35

#define HAP_INT0 GPIO_NUM_34

#define HAP_IO_ENABLE GPIO_NUM_25

#define HAP_PWM_LIVELLI  GPIO_NUM_26
#define HAP_POMPA1       GPIO_NUM_2
#define HAP_POMPA2       GPIO_NUM_12
#define HAP_LIVELLO1_H2O GPIO_NUM_32
#define HAP_LIVELLO2_H2O GPIO_NUM_33


#endif