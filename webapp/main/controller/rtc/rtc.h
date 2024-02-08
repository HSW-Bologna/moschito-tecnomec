#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

#include "model/model.h"
#include "view/view_types.h"

/* this includes messages sent by both 
 * the sender and the receiver */
typedef enum {
    MODEL_REPLACE_MESSAGE = 0,
    MODEL_UPDATE_MESSAGE,
    MESSAGE_TYPE_CONTROLLER,
} message_type_t;

void rtc_init(model_t *pmodel);

void rtc_send_model_update_message(
    const uint8_t *member_data,
    uint32_t member_data_size,
    uint32_t member_data_offset,
    uint16_t member_idx
);

void rtc_send_controller_message( view_controller_message_t *msg);
#endif
