#include <stdint.h>
#include <string.h> /* for memcpy */
#include "rtc.h"
#include "model_watcher.h"
#include "wsclient.h"
#include "model/model.h"
#include "model/model_descriptor.h"
#include "view/view.h"
#include "esp_log.h"

/* --- macros --- */

#define WS_TARGET_URL "ws://192.168.4.1/rtc"
// #define WS_TARGET_URL "ws://localhost:8765"

/* --- prototypes --- */

static void rtc_open_cb(void *arg);
static void rtc_message_recvd_cb(uint8_t *msg_data, uint32_t msg_size, void *arg);
static void rtc_error_cb(void *arg);
static void rtc_close_cb(unsigned short code, void *arg);

static void rtc_recvd_model_replace_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel);
static void rtc_recvd_model_update_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel);

/* --- variables --- */

/* websocket instance */
static ws_t ws;


static const char *TAG = "RTC";


/* --- public functions --- */

void rtc_init(model_t *pmodel) {
    /* create websocket connection */
    ws = wsclient_init(
        WS_TARGET_URL,
        rtc_open_cb, NULL,
        rtc_message_recvd_cb, (void *) pmodel,
        rtc_error_cb, NULL,
        rtc_close_cb, NULL
    );

    /* start watching model members */
    model_watcher_init(pmodel);

    ESP_LOGI(TAG, "RTC initialized");
}

/* --- rtc callbacks --- */

static void rtc_open_cb(void *arg) {
    (void) arg;
}

static void rtc_message_recvd_cb(uint8_t *msg_data, uint32_t msg_size, void *arg) {
    uint16_t msg_type;

    /* if the msg doesn't even contain the msg type, it's useless */
    if (msg_size < sizeof msg_type) {
        return;
    }

    model_t *pmodel = (model_t *) arg;

    msg_type = *((uint16_t *) msg_data);

    ESP_LOGI(TAG, "Received msg type: %hu", msg_type);

    /* remove the msg type header from the msg */
    msg_data += sizeof msg_type;
    msg_size -= sizeof msg_type;

    switch (msg_type) {
        case MODEL_REPLACE_MESSAGE:
            rtc_recvd_model_replace_message(msg_data, msg_size, pmodel);
            break;
        case MODEL_UPDATE_MESSAGE:
            rtc_recvd_model_update_message(msg_data, msg_size, pmodel);
            break;
        default:
            ESP_LOGE(TAG, "Invalid message type received: %hu", msg_type);
            break;
    }
}

static void rtc_error_cb(void *arg) {
    (void) arg;
}

static void rtc_close_cb(unsigned short code, void *arg) {
    (void) code;
    (void) arg;
}

/* --- send to server --- */

/*
 * Send MODEL_UPDATE_MESSAGE format
 *  uint16_t    message type
 *  uint16_t    model member idx
 *  uint32_t    model member offset
 *     x        model member data
 */
void rtc_send_model_update_message(
    const uint8_t *member_data,
    uint32_t member_data_size,
    uint32_t member_data_offset,
    uint16_t member_idx
) {
    ESP_LOGI(TAG, "Sending msg type: %hu", MODEL_UPDATE_MESSAGE);

    if (member_data_size == 0) {
        ESP_LOGE(TAG, "No member data to send on rtc_send_model_update_message()");
        return;
    }

    size_t msg_size = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + member_data_size;
    uint8_t msg_data[msg_size]; /* VLA */
    uint8_t *msg_data_ptr = msg_data;

    /* message type */
    *((uint16_t *) msg_data_ptr) = MODEL_UPDATE_MESSAGE;
    msg_data_ptr += sizeof(uint16_t);

    /* model member idx */
    *((uint16_t *) msg_data_ptr) = member_idx;
    msg_data_ptr += sizeof(uint16_t);

    /* model member offset */
    *((uint32_t *) msg_data_ptr) = member_data_offset;
    msg_data_ptr += sizeof(uint32_t);

    /* model member data */
    memcpy(msg_data_ptr, member_data, member_data_size);

    wsclient_send_binary(ws, msg_data, msg_size);
}


/*
 * Send MESSAGE_TYPE_CONTROLLER format
 *  uint16_t    message type
 *  uint16_t    controllermessage tag
 *     x        message data
 */
void rtc_send_controller_message( view_controller_message_t *msg) {
    ESP_LOGI(TAG, "Sending msg type: %hu", MESSAGE_TYPE_CONTROLLER);

    size_t msg_min_size = sizeof(uint16_t) + sizeof(uint16_t);
    size_t msg_max_size = msg_min_size + 1;
    uint8_t msg_data[msg_max_size]; /* VLA */

    *((uint16_t*)msg_data) = MESSAGE_TYPE_CONTROLLER;

    msg_data[2] = msg->code & 0xFF;

    size_t msg_size = msg_min_size;

    switch (msg->code) {
        case VIEW_CONTROLLER_MESSAGE_CODE_NOTHING:
            return; // avoid sending useless messages

        case VIEW_CONTROLLER_MESSAGE_CODE_TOGGLE_EROGATION: {
            msg_data[3] = msg->erogator;
            msg_size = msg_min_size + 1;
            break;
                                                            }

        case VIEW_CONTROLLER_MESSAGE_CODE_START_EROGATION:
        case VIEW_CONTROLLER_MESSAGE_CODE_STOP_EROGATION:
            break;

        case VIEW_CONTROLLER_MESSAGE_CODE_SAVE_RTC_TIME: {
            //utils_set_system_time(&msg->time_info);
            //TODO: serialize the time info
            break;
        }
    }

    wsclient_send_binary(ws, msg_data, msg_size);
}

/* --- received from server --- */

/*
 * Recvd MODEL_UPDATE_MESSAGE format
 *  uint16_t    model member idx
 *  uint32_t    model member offset
 *     x        model member data
 *
 * note: msg type is cut by the recvd callback
 */
static void rtc_recvd_model_update_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel) {
    /* check for required fields */
    if (msg_size < sizeof(uint16_t) + sizeof(uint32_t) + 1) {
        ESP_LOGE(TAG, "Missing required fields on rtc_recvd_model_update_message()");
        return;
    }

    uint8_t *msg_data_alias = msg_data;

    uint16_t model_member_idx = *((uint16_t *) msg_data_alias);
    model_member_t *model_member;
    if ((model_member = model_descriptor_get_member(model_member_idx)) == NULL) {
        ESP_LOGE(TAG, "Failed to find model member idx on rtc_recvd_model_update_message()");
        return;
    }
    msg_data_alias += sizeof model_member_idx;

    uint32_t model_member_offset = *((uint32_t *) msg_data_alias); 
    msg_data_alias += sizeof model_member_offset;

    uint32_t model_member_data_size = msg_size - (msg_data_alias - msg_data);

    /* check for overflow (i.e. sanitize offset and size) */
    if (// model_member_data_size < 1 || /* already included in the check at the beginning */
        model_member->size < model_member_offset + model_member_data_size
    ) {
        ESP_LOGE(TAG, "Failed overflow check on rtc_recvd_model_update_message()");
        return;
    }

    memcpy(
        (uint8_t *) ((unsigned char *) pmodel + model_member->offset) + model_member_offset,
        msg_data_alias,
        model_member_data_size
    );
    /* don't send the same message back to the server */
    model_watcher_trigger_member_silently(model_member_idx);

    view_refresh_current_page(pmodel);
}

/*
 * Recvd MODEL_REPLACE_MESSAGE format
 *  uint16_t    model member 1 idx
 *     x        model member 1 data
 *  ...
 *  ...
 *  uint16_t    model member n idx
 *     x        model member n data
 *
 * note: msg type is cut by the recvd callback
 */
static void rtc_recvd_model_replace_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel) {
    /* check for required fields */
    if (msg_size < sizeof(uint16_t) + 1) {
        ESP_LOGE(TAG, "Missing required fields on rtc_recvd_model_replace_message()");
        return;
    }

    size_t i = 0;

    while (i + sizeof(uint16_t) < msg_size) {
        uint16_t model_member_idx = *((uint16_t *) (msg_data + i));
        model_member_t *model_member;
        if ((model_member = model_descriptor_get_member(model_member_idx)) == NULL) {
            ESP_LOGE(TAG, "Failed to find model member idx on rtc_recvd_model_replace_message()");
            return; /* ignore the rest of the frame if failed */
        }
        i += sizeof model_member_idx;

        if (i + model_member->size > msg_size) {
            ESP_LOGE(TAG, "Invalid member data size on rtc_recvd_model_replace_message()");
            break;
        }

        memcpy(
            (unsigned char *) pmodel + model_member->offset,
            msg_data + i,
            model_member->size
        );
        i += model_member->size;
        /* don't send the same message back to the server */
        model_watcher_trigger_member_silently(model_member_idx);
    }

    /* todo: maybe add delayed refresh if possible */
    view_refresh_current_page(pmodel);
}
