#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "peripherals/buzzer.h"
#include "peripherals/i2c_devices.h"
#include "I2C/i2c_devices/io/MCP23008/mcp23008.h"
#include "esp_log.h"


static const char *TAG = "Controller";


void controller_init(model_t *pmodel) {
    // buzzer_beep(2, 50, 100);
#if 0
    uint8_t buffer = 0;
    mcp23008_get_gpio_direction_register(io_driver, &buffer);
    ESP_LOGI(TAG, "dir %X", buffer);
    mcp23008_set_gpio_direction_register(io_driver, 0x0F);
    mcp23008_set_gppu_register(io_driver, 0x0F);

    mcp23008_get_gpio_direction_register(io_driver, &buffer);
    ESP_LOGI(TAG, "dir %X", buffer);

    mcp23008_get_gppu_register(io_driver, &buffer);
    ESP_LOGI(TAG, "gppu %X", buffer);

    mcp23008_set_olat_register(io_driver, 0xFF);
    mcp23008_get_olat_register(io_driver, &buffer);
    ESP_LOGI(TAG, "olat %X", buffer);

    mcp23008_get_gpio_register(io_driver, &buffer);
    ESP_LOGI(TAG, "gpio %X", buffer);
    vTaskDelay(1000);

    mcp23008_set_olat_register(io_driver, 0x00);
    mcp23008_get_olat_register(io_driver, &buffer);
    ESP_LOGI(TAG, "olat %X", buffer);

    mcp23008_get_gpio_register(io_driver, &buffer);
    ESP_LOGI(TAG, "gpio %X", buffer);
    vTaskDelay(1000);

    mcp23008_set_olat_register(io_driver, 0xFF);
    mcp23008_get_olat_register(io_driver, &buffer);
    ESP_LOGI(TAG, "olat %X", buffer);

    mcp23008_get_gpio_register(io_driver, &buffer);
    ESP_LOGI(TAG, "gpio %X", buffer);
    vTaskDelay(1000);

    mcp23008_set_olat_register(io_driver, 0x00);
    mcp23008_get_olat_register(io_driver, &buffer);
    ESP_LOGI(TAG, "olat %X", buffer);

    mcp23008_get_gpio_register(io_driver, &buffer);
    ESP_LOGI(TAG, "gpio %X", buffer);
    vTaskDelay(1000);
    #endif

    view_change_page(pmodel, &page_main);
}


void controller_process_message(model_t *pmodel, view_controller_message_t *msg) {
    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            break;
    }
}


void controller_manage(model_t *pmodel) {
    (void)pmodel;
}