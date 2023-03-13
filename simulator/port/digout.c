#include <stdio.h>
#include "peripherals/digout.h"


void digout_update(digout_t digout, uint8_t value) {
    printf("Setting %s %s\n",
           digout == DIGOUT_PUMP         ? "POMPA"
           : digout == DIGOUT_EROGATOR_1 ? "ER1"
                                         : "ER2",
           value ? "ON" : "OFF");
}