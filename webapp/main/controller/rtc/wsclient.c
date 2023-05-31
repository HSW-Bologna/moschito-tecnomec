#include <stdint.h>
#include <string.h> /* for memcpy */
#include <emscripten/websocket.h>
#include "wsclient.h"
#include "esp_log.h"
#include "gel/collections/listx.h"

typedef struct {
    wsclient_nested_onopen_cb_t nested_ocb;
    void *nested_ocb_arg;
} wsclient_onopen_cb_arg_t;

typedef struct {
    wsclient_nested_onmessage_cb_t nested_mcb;
    void *nested_mcb_arg;
} wsclient_onmessage_cb_arg_t;

typedef struct {
    wsclient_nested_onerror_cb_t nested_ecb;
    void *nested_ecb_arg;
} wsclient_onerror_cb_arg_t;

typedef struct {
    wsclient_nested_onclose_cb_t nested_ccb;
    void *nested_ccb_arg;
} wsclient_onclose_cb_arg_t;

typedef struct {
    ws_t ws;
    uint8_t *data;
    uint32_t size;
    struct list_head list;
} msg_queue_t;

static EM_BOOL wsclient_onopen_cb(int event_type, const EmscriptenWebSocketOpenEvent *ws_event, void *arg);
static EM_BOOL wsclient_onmessage_cb(int event_type, const EmscriptenWebSocketMessageEvent *ws_event, void *arg);
static EM_BOOL wsclient_onerror_cb(int event_type, const EmscriptenWebSocketErrorEvent *ws_event, void *arg);
static EM_BOOL wsclient_onclose_cb(int event_type, const EmscriptenWebSocketCloseEvent *ws_event, void *arg);

/* head of the queue of messages (sentinel node)
 * this queue is used for deferring sent messages
 * when the websocket has not yet connected
 * (i.e. it is in the connecting state)
 */
static struct list_head msg_queue_head;


static const char *TAG = "WSClient";


ws_t wsclient_init(
    const char *url,
    wsclient_nested_onopen_cb_t nested_ocb, void *nested_ocb_arg,
    wsclient_nested_onmessage_cb_t nested_mcb, void *nested_mcb_arg,
    wsclient_nested_onerror_cb_t nested_ecb, void *nested_ecb_arg,
    wsclient_nested_onclose_cb_t nested_ccb, void *nested_ccb_arg
) {
    static wsclient_onopen_cb_arg_t ocb_arg;
    static wsclient_onmessage_cb_arg_t mcb_arg;
    static wsclient_onerror_cb_arg_t ecb_arg;
    static wsclient_onclose_cb_arg_t ccb_arg;

    EmscriptenWebSocketCreateAttributes ws_attrs = {
        url,
        NULL,
        /* todo: try to create a separate thread and test it */
        EM_TRUE // create on main thread
    };

    /* create WebSocket connection */
    ws_t ws = emscripten_websocket_new(&ws_attrs);

    ESP_LOGI(TAG, "Opening connection to %s", url);

    if (nested_ocb != NULL) {
        ocb_arg = (wsclient_onopen_cb_arg_t) {nested_ocb, nested_ocb_arg};
        emscripten_websocket_set_onopen_callback(ws, &ocb_arg, wsclient_onopen_cb);
    }
    if (nested_mcb != NULL) {
        mcb_arg = (wsclient_onmessage_cb_arg_t) {nested_mcb, nested_mcb_arg};
        emscripten_websocket_set_onmessage_callback(ws, &mcb_arg, wsclient_onmessage_cb);
    }
    if (nested_ecb != NULL) {
        ecb_arg = (wsclient_onerror_cb_arg_t) {nested_ecb, nested_ecb_arg};
        emscripten_websocket_set_onerror_callback(ws, &ecb_arg, wsclient_onerror_cb);
    }
    if (nested_ccb != NULL) {
        ccb_arg = (wsclient_onclose_cb_arg_t) {nested_ccb, nested_ccb_arg};
        emscripten_websocket_set_onclose_callback(ws, &ccb_arg, wsclient_onclose_cb);
    }

    /* init msg queue */
    INIT_LIST_HEAD(&msg_queue_head);

    return ws;
}

ws_ready_state_t wsclient_get_ready_state(ws_t ws) {
    EMSCRIPTEN_RESULT rc;
    unsigned short ws_state;

    rc = emscripten_websocket_get_ready_state(ws, &ws_state);

    if (rc != EMSCRIPTEN_RESULT_SUCCESS) {
        ESP_LOGE(TAG, "Failed to emscripten_websocket_get_ready_state(): %d", rc);
        return WS_EMS_ERROR_STATE;
    }

    return (ws_ready_state_t) ws_state;
}

int wsclient_send_binary(ws_t ws, void *msg_data, size_t msg_size) {
    EMSCRIPTEN_RESULT rc;

    if (wsclient_get_ready_state(ws) == WS_CONNECTING_STATE) {
        /* put message on a queue */
        msg_queue_t *msg = malloc(sizeof(msg_queue_t));
        msg->ws = ws;
        msg->data = malloc(msg_size);
        memcpy(msg->data, msg_data, msg_size);
        msg->size = msg_size;
        list_add_tail(&msg->list, &msg_queue_head);
        return -2; // connection is not currently ready
    }

    /* note: msg_data is copied in a buffer by emscripten
     * so there's no need to keep alive a reference of the data */
    rc = emscripten_websocket_send_binary(ws, msg_data, msg_size);

    if (rc != EMSCRIPTEN_RESULT_SUCCESS) {
        ESP_LOGE(TAG, "Failed to emscripten_websocket_send_binary(): %d", rc);
        return -1;
    }

    return 0;
}

/*
 * Connection opened event
 */
static EM_BOOL wsclient_onopen_cb(int event_type, const EmscriptenWebSocketOpenEvent *ws_event, void *arg) {
    (void) event_type;

    wsclient_onopen_cb_arg_t *ocb_arg = (wsclient_onopen_cb_arg_t *) arg;

    ESP_LOGI(TAG, "Connection established (%d)", ws_event->socket);

    msg_queue_t *msg;
    struct list_head *pos = msg_queue_head.next;

    /* send all the messages in queue
     * (the messages sent during the connecting state) */
    while (pos != &msg_queue_head) {
        msg = container_of(pos, msg_queue_t, list);
        if (msg->ws == ws_event->socket) {
            wsclient_send_binary(msg->ws, msg->data, msg->size);
            pos = pos->next;
            list_del(pos->prev);
            free(msg->data);
            free(msg);
        }
    }

    ocb_arg->nested_ocb(ocb_arg->nested_ocb_arg);

    return EM_TRUE;
}

/*
 * Listen for received data event
 */
static EM_BOOL wsclient_onmessage_cb(int event_type, const EmscriptenWebSocketMessageEvent *ws_event, void *arg) {
    (void) event_type;

    wsclient_onmessage_cb_arg_t *mcb_arg = (wsclient_onmessage_cb_arg_t *) arg;
    ESP_LOGI(TAG, "Received %u bytes (%d)", ws_event->numBytes, ws_event->socket);
    mcb_arg->nested_mcb(ws_event->data, ws_event->numBytes, mcb_arg->nested_mcb_arg);

    return EM_TRUE;
}

/*
 * Error event
 */
static EM_BOOL wsclient_onerror_cb(int event_type, const EmscriptenWebSocketErrorEvent *ws_event, void *arg) {
    (void) event_type;

    wsclient_onerror_cb_arg_t *ecb_arg = (wsclient_onerror_cb_arg_t *) arg;
    ESP_LOGI(TAG, "Connection closed due to an error (%d)", ws_event->socket);
    ecb_arg->nested_ecb(ecb_arg->nested_ecb_arg);

    return EM_TRUE;
}

/*
 * Connection closed event
 */
static EM_BOOL wsclient_onclose_cb(int event_type, const EmscriptenWebSocketCloseEvent *ws_event, void *arg) {
    (void) event_type;
    
    wsclient_onclose_cb_arg_t *ccb_arg = (wsclient_onclose_cb_arg_t *) arg;
    ESP_LOGI(TAG, "Connection closed (%d), code: %hu, reason: \"%s\"", ws_event->socket, ws_event->code, ws_event->reason);
    ccb_arg->nested_ccb(ws_event->code, ccb_arg->nested_ccb_arg);

    return EM_TRUE;
}
