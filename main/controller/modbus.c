#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lightmodbus/master.h"
#include "peripherals/hardwareprofile.h"
#include "esp_log.h"
#include "config/app_config.h"
#include <sys/types.h>
#include "lightmodbus/lightmodbus.h"
#include "lightmodbus/master_func.h"
#include "gel/timer/timecheck.h"
#include "utils/utils.h"
#include "modbus.h"
#include "peripherals/rs232.h"


#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)

#define BASE_TASK_SIZE       1024
#define SLAVE_DEVICE_ADDRESS 2

#define MODBUS_MESSAGE_QUEUE_SIZE     512
#define MODBUS_QUERY_INTERVAL         500
#define MODBUS_TIMEOUT                50
#define MODBUS_MAX_PACKET_SIZE        256
#define MODBUS_COMMUNICATION_ATTEMPTS 3

#define MODBUS_HOLDING_REGISTER_ADDRESS 0

#define MODBUS_AUTO_COMMISSIONING_DONE_BIT 0x01

struct __attribute__((packed)) task_message {
    uint8_t address;
    union {
        struct {
            int value;
            int index;
        };
    };
};

static void modbus_task(void *args);

static const char   *TAG       = "Modbus";
static QueueHandle_t messageq  = NULL;
static QueueHandle_t responseq = NULL;
static TaskHandle_t  task      = NULL;

void modbus_init(void) {
    static StaticQueue_t static_queue1;
    static uint8_t       queue_buffer1[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(struct task_message)] = {0};
    messageq =
        xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(struct task_message), queue_buffer1, &static_queue1);

    static StaticQueue_t static_queue2;
    static uint8_t       queue_buffer2[MODBUS_MESSAGE_QUEUE_SIZE * sizeof(modbus_response_t)] = {0};
    responseq = xQueueCreateStatic(MODBUS_MESSAGE_QUEUE_SIZE, sizeof(modbus_response_t), queue_buffer2, &static_queue2);

    static uint8_t      task_stack[BASE_TASK_SIZE * 4] = {0};
    static StaticTask_t static_task;
    task = xTaskCreateStatic(modbus_task, TAG, sizeof(task_stack), NULL, 5, task_stack, &static_task);
}


static ModbusError dataCallback(const ModbusMaster *master, const ModbusDataCallbackArgs *args) {
    modbus_response_t *response = modbusMasterGetUserPointer(master);
    printf("Received data from %d, reg: %d, value: %d\n", args->address, args->index, args->value);
    if (response != NULL) {}
    return MODBUS_OK;
}

static ModbusError masterExceptionCallback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                           ModbusExceptionCode code) {
    printf("Received exception (function %d) from slave %d code %d\n", function, address, code);
    return MODBUS_OK;
}


static void modbus_task(void *args) {
    (void)args;
    ModbusMaster    master;
    ModbusErrorInfo err = modbusMasterInit(&master,
                                           dataCallback,                // Callback for handling incoming data
                                           masterExceptionCallback,     // Exception callback (optional)
                                           modbusDefaultAllocator,      // Memory allocator used to allocate request
                                           modbusMasterDefaultFunctions,        // Set of supported functions
                                           modbusMasterDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusMasterInit() failed");
    struct task_message message    = {0};
    modbus_response_t   error_resp = {.code = MODBUS_RESPONSE_ERROR};

    ESP_LOGI(TAG, "Task starting");
    for (;;) {
        xTaskNotifyStateClear(task);
        // uart_flush_input(MB_PORTNUM);

        if (xQueueReceive(messageq, &message, pdMS_TO_TICKS(100))) {
            error_resp.address = message.address;
            (void)error_resp;
            modbusMasterSetUserPointer(&master, NULL);

            vTaskDelay(pdMS_TO_TICKS(MODBUS_TIMEOUT));
        }
    }
    vTaskDelete(NULL);
}
