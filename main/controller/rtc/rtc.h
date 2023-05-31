#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

#include "model/model.h"

/* this includes messages sent by both 
 * the sender and the receiver */
typedef enum {
    MODEL_REPLACE_MESSAGE = 0,
    MODEL_UPDATE_MESSAGE
} message_type_t;

void rtc_initt(model_t *pmodel);
void rtc_message_recvd_handler(uint8_t *msg_data, uint32_t msg_size, model_t* pmodel);

void rtc_send_model_update_message(
    const uint8_t *member_data,
    uint32_t member_data_size,
    uint32_t member_data_offset,
    uint16_t member_idx
);
void rtc_send_model_replace_message(model_t *pmodel);
#endif
