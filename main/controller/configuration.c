#include "peripherals/storage.h"
#include "model/model.h"
#include "esp_log.h"


static const char *TAG = "Configuration";

const char *CONFIGURATION_LANGUAGE_KEY               = "LANG";
const char *CONFIGURATION_BRIGHTNESS_KEY             = "BRIGHTNESS";
const char *CONFIGURATION_VOLUME_KEY                 = "BRIGHTNESS";
const char *CONFIGURATION_EROGATION_1_PERCENTAGE_KEY = "ER1PERC";
const char *CONFIGURATION_EROGATION_2_PERCENTAGE_KEY = "ER2PERC";

const char *CONFIGURATION_SCHEDULER_ENTRIES_KEYS[NUM_EROGATORS][NUM_PROGRAMS] = {
    {"ER1EN0", "ER1EN1", "ER1EN2", "ER1EN3", "ER1EN4", "ER1EN5"},
    {"ER2EN0", "ER2EN1", "ER2EN2", "ER2EN3", "ER2EN4", "ER2EN5"},
};

const char *CONFIGURATION_SCHEDULER_WM_KEYS[NUM_EROGATORS][NUM_PROGRAMS] = {
    {"WM1EN0", "WM1EN1", "WM1EN2", "WM1EN3", "WM1EN4", "WM1EN5"},
    {"WM2EN0", "WM2EN1", "WM2EN2", "WM2EN3", "WM2EN4", "WM2EN5"},
};

const char *CONFIGURATION_SCHEDULER_ACTIVE_TIME_KEYS[NUM_EROGATORS][NUM_PROGRAMS] = {
    {"AT1EN0", "AT1EN1", "AT1EN2", "AT1EN3", "AT1EN4", "AT1EN5"},
    {"AT2EN0", "AT2EN1", "AT2EN2", "AT2EN3", "AT2EN4", "AT2EN5"},
};

const char *CONFIGURATION_SCHEDULER_PAUSE_TIME_KEYS[NUM_EROGATORS][NUM_PROGRAMS] = {
    {"PT1EN0", "PT1EN1", "PT1EN2", "PT1EN3", "PT1EN4", "PT1EN5"},
    {"PT2EN0", "PT2EN1", "PT2EN2", "PT2EN3", "PT2EN4", "PT2EN5"},
};


void configuration_load(model_t *pmodel) {
    storage_load_uint16(&pmodel->configuration.language, (char *)CONFIGURATION_LANGUAGE_KEY);
    storage_load_uint8(&pmodel->configuration.brightness, (char *)CONFIGURATION_VOLUME_KEY);
    storage_load_uint8(&pmodel->configuration.volume, (char *)CONFIGURATION_VOLUME_KEY);
    storage_load_uint8(&pmodel->configuration.erogator_percentages[EROGATOR_1],
                       (char *)CONFIGURATION_EROGATION_1_PERCENTAGE_KEY);
    storage_load_uint8(&pmodel->configuration.erogator_percentages[EROGATOR_2],
                       (char *)CONFIGURATION_EROGATION_2_PERCENTAGE_KEY);

    for (size_t i = 0; i < NUM_EROGATORS; i++) {
        for (size_t j = 0; j < NUM_PROGRAMS; j++) {
            uint8_t buffer[SCHEDULER_ENTRY_SERIALIZED_SIZE] = {0};
            storage_load_blob(buffer, sizeof(buffer), (char *)CONFIGURATION_SCHEDULER_ENTRIES_KEYS[i][j]);
            scheduler_entry_deserialize(&pmodel->configuration.schedulers[i].entries[j], buffer);

            storage_load_uint8(&pmodel->configuration.working_modes[i][j],
                               (char *)CONFIGURATION_SCHEDULER_WM_KEYS[i][j]);
            storage_load_uint32(&pmodel->configuration.active_seconds[i][j],
                                (char *)CONFIGURATION_SCHEDULER_ACTIVE_TIME_KEYS[i][j]);
            storage_load_uint32(&pmodel->configuration.pause_seconds[i][j],
                                (char *)CONFIGURATION_SCHEDULER_PAUSE_TIME_KEYS[i][j]);
        }
    }
}