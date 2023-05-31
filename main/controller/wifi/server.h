#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <esp_http_server.h>
#include "model/model.h"

typedef struct {
    uint8_t *data;
    uint32_t size;
} rtc_ws_msg_t;

void webserver_init(void);
httpd_handle_t start_webserver(model_t *pmodel);
esp_err_t stop_webserver(httpd_handle_t server);
int rtc_ws_get_next_msg(rtc_ws_msg_t *ws_msg);
void rtc_ws_send_binary(void *msg_data, size_t msg_size);

#endif
