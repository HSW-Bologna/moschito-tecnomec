#include <driver/i2c.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "lvgl_helpers.h"
#include "lvgl_i2c/i2c_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/buzzer.h"
#include "peripherals/i2c_devices.h"
#include "i2c_ports/esp-idf/esp_idf_i2c_port.h"


static const char *TAG = "Main";


void app_main(void) {
    model_t model;

    lvgl_i2c_init(I2C_NUM_0);
    lvgl_driver_init();
    buzzer_init();
    i2c_devices_init();

    esp_idf_i2c_scanner(I2C_NUM_0);

    model_init(&model);
    view_init(&model, disp_driver_flush, ft6x36_read);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_manage(&model);
        controller_manage(&model);

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
