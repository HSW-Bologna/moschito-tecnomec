#include <driver/i2c.h>
#include <driver/gpio.h>
#include "hardwareprofile.h"
#include "i2c_devices.h"
#include "i2c_devices/io/MCP23008/mcp23008.h"
#include "i2c_ports/esp-idf/esp_idf_i2c_port.h"


i2c_driver_t io_driver = {
    .device_address = MCP23008_DEFAULT_ADDR,
    .i2c_transfer   = esp_idf_i2c_port_transfer,
    .arg            = I2C_NUM_0,
};


void i2c_devices_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(HAP_IO_ENABLE),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(HAP_IO_ENABLE, 1));
}