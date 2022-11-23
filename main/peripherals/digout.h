#ifndef DIGOUT_H_INCLUDED
#define DIGOUT_H_INCLUDED


#include <stdint.h>


#define DIGOUT_SET(digout)   digout_update(digout, 1)
#define DIGOUT_CLEAR(digout) digout_update(digout, 0)


typedef enum {
    DIGOUT_EROGATOR_1 = 0,
    DIGOUT_EROGATOR_2,
    DIGOUT_PUMP,
} digout_t;


void digout_init(void);
void digout_update(digout_t digout, uint8_t value);


#endif