#include <stdint.h>
#include "rtc.h"
#include "model_watcher.h"
#include "model/model.h"
#include "model/model_descriptor.h"
#include "view/view.h"
#include "esp_log.h"
#include "controller/wifi/server.h"

/* --- prototypes --- */

static void rtc_recvd_model_update_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel);


static const char *TAG = "RTC";


/* --- public functions --- */

void rtc_initt(model_t *pmodel) {
    /* start watching model members */
    model_watcher_init(pmodel);

    ESP_LOGI(TAG, "RTC initialized");
}

void rtc_message_recvd_handler(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel) {
    uint16_t msg_type;

    /* if the msg doesn't even contain the msg type, it's useless */
    if (msg_size < sizeof msg_type) {
        return;
    }

    msg_type = *((uint16_t*) msg_data);

    ESP_LOGI(TAG, "Received msg type: %hu", msg_type);

    /* remove the msg type header from the msg */
    msg_data += sizeof msg_type;
    msg_size -= sizeof msg_type;

    switch (msg_type) {
        case MODEL_UPDATE_MESSAGE:
            rtc_recvd_model_update_message(msg_data, msg_size, pmodel);
            break;
        case MODEL_REPLACE_MESSAGE:
            ESP_LOGE(TAG, "Not supported type to receive: %hu", msg_type);
            break;
        default:
            ESP_LOGW(TAG, "Invalid message type received: %hu", msg_type);
            break;
    }
}

/* --- send to client --- */

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
    uint8_t msg_data[msg_size];
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

    rtc_ws_send_binary(msg_data, msg_size);
}

/*
 * Send MODEL_REPLACE_MESSAGE format
 *  uint16_t    message type
 *  uint16_t    model member 1 idx
 *     x        model member 1 data
 *  ...
 *  ...
 *  uint16_t    model member n idx
 *     x        model member n data
 */
void rtc_send_model_replace_message(model_t *pmodel) {
#define BUF_SIZE 100 /* size must be > the max size of the model members */
    ESP_LOGI(TAG, "Sending msg type: %hu", MODEL_REPLACE_MESSAGE);

    uint8_t buf[BUF_SIZE];
    size_t i = 0;
    uint16_t model_member_idx = 0;
    model_member_t *model_member;

    /* message type */
    *((uint16_t *) buf) = MODEL_REPLACE_MESSAGE;
    i += sizeof(uint16_t);

    /* send multiple replace messages using always the same buffer */
    while ((model_member = model_descriptor_get_member(model_member_idx)) != NULL) {
        if (i + sizeof(uint16_t) + model_member->size > BUF_SIZE) {
            rtc_ws_send_binary(buf, i);
            i = 0;

            /* message type */
            *((uint16_t *) buf) = MODEL_REPLACE_MESSAGE;
            i += sizeof(uint16_t);
        }

        /* model member idx */
        *((uint16_t *) (buf + i)) = model_member_idx;
        i += sizeof(uint16_t);

        /* model member data */
        memcpy(buf + i, (unsigned char *) pmodel + model_member->offset, model_member->size);
        i += model_member->size;

        ++model_member_idx;
    }

    rtc_ws_send_binary(buf, i);
#undef BUF_SIZE
}

/* --- received from client --- */

/*
 * Recvd MODEL_UPDATE_MESSAGE format
 *  uint16_t    model member idx
 *  uint32_t    model member offset
 *     x        model member data
 *
 * note: msg type is cut by the recvd handler
 */
static void rtc_recvd_model_update_message(uint8_t *msg_data, uint32_t msg_size, model_t *pmodel) {
    /* check for required fields */
    if (msg_size < sizeof(uint16_t) + sizeof(uint32_t) + 1) {
        ESP_LOGE(TAG, "Missing required fields on rtc_recvd_model_update_message()");
        return;
    }

    uint8_t* msg_data_alias = msg_data;

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
    // model_watcher_trigger_member_silently(model_member_idx);
    view_refresh_current_page(pmodel);
}
