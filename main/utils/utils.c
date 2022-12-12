#include <sys/time.h>
#include <time.h>
#include "esp_log.h"
#include "utils.h"


static const char *TAG = "Utils";


void utils_set_system_time(struct tm *systm) {
    ESP_LOGI(TAG, "Setting system time to %02i:%02i %02i/%02i/%02i", systm->tm_hour, systm->tm_min, systm->tm_mday,
             systm->tm_mon + 1, systm->tm_year);

    struct timeval timeval;
    timeval.tv_sec  = mktime(systm);
    timeval.tv_usec = 0;
    settimeofday(&timeval, NULL);
}