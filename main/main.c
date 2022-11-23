#include <driver/i2c.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "lvgl_helpers.h"
#include "lvgl_i2c/i2c_manager.h"
#include "model/model.h"
#include "peripherals/button.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "peripherals/buzzer.h"
#include "peripherals/i2c_devices.h"
#include "peripherals/digout.h"
#include "peripherals/hardwareprofile.h"
#include "controller/modbus.h"
#include "peripherals/livelli.h"
#include "gel/keypad/keypad.h"
#include "peripherals/button.c"
#include "utils/utils.h"


static const char *TAG = "Main";


void app_main(void) {
    model_t model;

    lvgl_i2c_init(I2C_NUM_0);
    lvgl_driver_init();
    digout_init();
    buzzer_init();
    i2c_devices_init();
    modbus_init();
    livelli_init();
    button_init();

    model_init(&model);
    view_init(&model, disp_driver_flush, ft6x36_read);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");
    for (;;) {
        controller_gui_manage(&model);
        controller_manage(&model);

        /*
        uint16_t buffer1 = 0;
        uint16_t buffer2 = 0;
        livelli_get_values(&buffer1, &buffer2);
        ESP_LOGI(TAG, "letto: %i, %i", buffer1, buffer2 );
        */

        keypad_update_t update = button_manage(get_millis());
        if (update.event == KEY_CLICK) {
            ESP_LOGI(TAG, "premuto bottone");
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
