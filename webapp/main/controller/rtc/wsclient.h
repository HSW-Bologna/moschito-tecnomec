#ifndef WEBSOCKET_CLIENT_H_INCLUDED
#define WEBSOCKET_CLIENT_H_INCLUDED

#include <emscripten/websocket.h>

// https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/readyState
typedef enum {
    WS_EMS_ERROR_STATE = -1, // custom state
    WS_CONNECTING_STATE = 0,
    WS_OPEN_STATE,
    WS_CLOSING_STATE,
    WS_CLOSED_STATE,
} ws_ready_state_t;

typedef EMSCRIPTEN_WEBSOCKET_T ws_t; // int

typedef void (*wsclient_nested_onopen_cb_t)(void *arg);
typedef void (*wsclient_nested_onmessage_cb_t)(uint8_t *msg_data, uint32_t msg_size, void *arg);
typedef void (*wsclient_nested_onerror_cb_t)(void *arg);
typedef void (*wsclient_nested_onclose_cb_t)(unsigned short code, void *arg);

ws_t wsclient_init(
    const char *url,
    wsclient_nested_onopen_cb_t nested_ocb, void *nested_ocb_arg,
    wsclient_nested_onmessage_cb_t nested_mcb, void *nested_mcb_arg,
    wsclient_nested_onerror_cb_t nested_ecb, void *nested_ecb_arg,
    wsclient_nested_onclose_cb_t nested_ccb, void *nested_ccb_arg
);
ws_ready_state_t wsclient_get_ready_state(ws_t ws);
int wsclient_send_binary(ws_t ws, void *msg_data, size_t msg_size);

#endif
