#include "driver/uart.h"
#include "driver/gpio.h"
#include "rs232.h"
#include "hardwareprofile.h"


#define MB_PORTNUM 1
// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT (3)     // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks


void rs232_init(void) {
    uart_config_t uart_config = {
        .baud_rate           = 115200,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(MB_PORTNUM, &uart_config));

    uart_set_pin(MB_PORTNUM, HAP_TX1, HAP_RX1, -1, -1);
    ESP_ERROR_CHECK(uart_driver_install(MB_PORTNUM, 512, 512, 10, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(MB_PORTNUM, UART_MODE_UART));
    ESP_ERROR_CHECK(uart_set_rx_timeout(MB_PORTNUM, ECHO_READ_TOUT));
}


void rs232_write(const uint8_t *data, size_t len) {
    uart_write_bytes(MB_PORTNUM, data, len);
}


int rs232_read(uint8_t *buffer, size_t len, unsigned long ms) {
    return uart_read_bytes(MB_PORTNUM, buffer, len, pdMS_TO_TICKS(ms));
}


void rs232_flush(void) {
    uart_flush(MB_PORTNUM);
}