#include "livelli.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/projdefs.h"
#include "hal/adc_types.h"
#include "hal/gpio_types.h"
#include "hardwareprofile.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include <sys/_stdint.h>

#define READ_PERIOD 50
#define NUM_READINGS 5
static uint16_t livello1[NUM_READINGS];
static uint16_t livello2[NUM_READINGS];

static size_t avarage_index=0;
static size_t first_loop=1;

static TimerHandle_t timer_read = NULL;


static void livelli_read(void *arg) { //da fare periodicamente
    (void)arg;
    uint16_t lvl1 = adc1_get_raw(ADC1_CHANNEL_4);
    uint16_t lvl2 = adc1_get_raw(ADC1_CHANNEL_5);
    livello1[avarage_index]=lvl1;
    livello2[avarage_index]=lvl2;
    if (avarage_index==NUM_READINGS-1) {
        first_loop=0;
    }
    avarage_index=(avarage_index+1)%NUM_READINGS;  
}

void livelli_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

    timer_read = xTimerCreate("read speed", pdMS_TO_TICKS(READ_PERIOD), pdTRUE, NULL, livelli_read);
    xTimerStart(timer_read, portMAX_DELAY);
}

void livelli_get_values(uint16_t *lvl1,uint16_t *lvl2 ) {
    size_t i;
    uint16_t sum1=0;
    uint16_t sum2=0;
    size_t num_readings = first_loop ? avarage_index : NUM_READINGS;
    for (i=0;i<num_readings;i++) {
        sum1+=livello1[i];
        sum2+=livello2[i];
    }
    if (num_readings==0) {
        *lvl1=0;
        *lvl2=0;
    }
    else {
        *lvl1 = (uint16_t) (sum1/num_readings);
        *lvl2 = (uint16_t) (sum2/num_readings);
    }
}
