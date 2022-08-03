#include "peripherals/button.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "peripherals/hardwareprofile.h"
#include "gel/keypad/keypad.h"


static int               ignore_events = 0;


static keypad_key_t keyboard[] = {
    KEYPAD_KEY(0x01, BUTTON),
    KEYPAD_NULL_KEY,
};

void button_init(void) {
    gpio_config_t io_conf_input = {
        // interrupt of falling edge
        .intr_type = GPIO_INTR_DISABLE,
        // bit mask of the pins
        .pin_bit_mask = (1ULL << HAP_ON_OFF),
        // set as input mode
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = 0,
        .pull_down_en = 0,
    };
    gpio_config(&io_conf_input);
}


void button_reset(void) {
    ignore_events = 1;
}


unsigned int button_read(void) {
    return gpio_get_level(HAP_ON_OFF);
}


keypad_update_t button_manage(unsigned long ts) {
    unsigned int keymap = button_read();
    if (ignore_events) {
        if (keymap == 0) {
            ignore_events = 0;
            keypad_reset_keys(keyboard);
        }
        return (keypad_update_t){.event = KEY_NOTHING};
    } else {
        return keypad_routine(keyboard, 40, 1500, 100, ts, keymap);
    }
}