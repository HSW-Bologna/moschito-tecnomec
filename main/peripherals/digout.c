#include <driver/gpio.h>
#include "digout.h"
#include "hardwareprofile.h"
#include "i2c_devices.h"
#include "I2C/i2c_devices/io/MCP23008/mcp23008.h"
#include "esp_log.h"


static const char *TAG = "Digout";


void digout_init(void) {
    gpio_config_t config = {
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT(HAP_POMPA1) | BIT(HAP_POMPA2),
    };
    ESP_ERROR_CHECK(gpio_config(&config));

    mcp23008_set_gpio_direction_register(io_driver, 0x0F);
    mcp23008_set_gppu_register(io_driver, 0x0F);

    DIGOUT_CLEAR(DIGOUT_PUMP);

    ESP_LOGI(TAG, "Initialized");
}


void digout_update(digout_t digout, uint8_t value) {
    switch (digout) {
        case DIGOUT_EROGATOR_1:
        case DIGOUT_EROGATOR_2: {
            const gpio_num_t gpios[2] = {HAP_POMPA1, HAP_POMPA2};
            ESP_ERROR_CHECK(gpio_set_level(gpios[digout], value > 0));
            break;
        }

        case DIGOUT_PUMP: {
            const mcp23008_gpio_t gpios[1] = {MCP23008_GPIO_7};
            mcp23008_set_gpio_level(io_driver, gpios[digout - DIGOUT_PUMP], value > 0);
            break;
        }
    }
}
