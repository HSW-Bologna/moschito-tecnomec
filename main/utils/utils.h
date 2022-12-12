#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#define get_millis() (xTaskGetTickCount() * portTICK_PERIOD_MS)

void utils_set_system_time(struct tm *systm);

#endif
