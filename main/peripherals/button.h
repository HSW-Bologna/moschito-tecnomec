#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stdint.h>
#include "gel/keypad/keypad.h"

typedef enum {
    BUTTON_NONE  = 0,
    BUTTON = 1,
} button_t;

void button_init(void);
unsigned int button_read(void);
void button_reset(void);
keypad_update_t button_manage(unsigned long ts);

#endif