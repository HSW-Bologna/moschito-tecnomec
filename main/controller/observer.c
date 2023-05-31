#include "configuration.h"
#include "watcher.h" /* /components/c-watcher */
#include "peripherals/storage.h"
#include "utils/utils.h"
#include "erogator/erogator.h"


#define NUM_OBSERVED_VARIABLES (6 + (NUM_PROGRAMS * NUM_EROGATORS) * 4)


static void save_configuration_entry(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg);
static void save_schedule_entry(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg);
static void scheduler_value_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg);


static int       scheduler_value[3]                    = {0};
static watcher_t watcher;


void observer_init(model_t *pmodel) {
    watcher_init(&watcher, pmodel);

    WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.language, (void *) save_configuration_entry,
                      (void *) CONFIGURATION_LANGUAGE_KEY);
    WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.brightness, (void *) save_configuration_entry,
                      (void *) CONFIGURATION_BRIGHTNESS_KEY);
    WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.volume, (void *) save_configuration_entry,
                      (void *) CONFIGURATION_VOLUME_KEY);
    WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.erogator_percentages[EROGATOR_1], (void *) save_configuration_entry,
                      (void *) CONFIGURATION_EROGATION_1_PERCENTAGE_KEY);
    WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.erogator_percentages[EROGATOR_2], (void *) save_configuration_entry,
                      (void *) CONFIGURATION_EROGATION_2_PERCENTAGE_KEY);
    WATCHER_ADD_ENTRY(&watcher, scheduler_value, (void *) scheduler_value_cb, NULL);

    for (size_t i = 0; i < NUM_EROGATORS; ++i) {
        for (size_t j = 0; j < NUM_PROGRAMS; ++j) {
            WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.schedulers[i].entries[j], (void *) save_schedule_entry,
                              (void *) CONFIGURATION_SCHEDULER_ENTRIES_KEYS[i][j]);
            WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.working_modes[i][j], (void *) save_configuration_entry,
                              (void *) CONFIGURATION_SCHEDULER_WM_KEYS[i][j]);
            WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.active_seconds[i][j], (void *) save_configuration_entry,
                              (void *) CONFIGURATION_SCHEDULER_ACTIVE_TIME_KEYS[i][j]);
            WATCHER_ADD_ENTRY(&watcher, &pmodel->configuration.pause_seconds[i][j], (void *) save_configuration_entry,
                              (void *) CONFIGURATION_SCHEDULER_PAUSE_TIME_KEYS[i][j]);
        }
    }
}


void observer_observe(model_t *pmodel) {
    erogator_t erogator = scheduler_value[1];
    scheduler_value[0]  = model_get_scheduler_active_erogator(pmodel, &erogator);
    scheduler_value[1]  = erogator;
    scheduler_value[2]  = model_is_erogation_stopped(pmodel);

    watcher_watch(&watcher, get_millis());
}


static void save_configuration_entry(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg) {
    (void) old_buffer;
    (void) user_ptr;
    (void) arg;

    char *configuration_key = (char *) arg;

    switch (size) {
        case 1:
            storage_save_uint8((uint8_t *) memory, configuration_key);
            break;
        case 2:
            storage_save_uint16((uint16_t *) memory, configuration_key);
            break;
        case 4:
            storage_save_uint32((uint32_t *) memory, configuration_key);
            break;
        default:
            break;
    }
}


static void save_schedule_entry(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg) {
    uint8_t buffer[SCHEDULER_ENTRY_SERIALIZED_SIZE] = {0};
    scheduler_entry_serialize(buffer, memory);
    storage_save_blob(buffer, sizeof(buffer), arg);
}


static void scheduler_value_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg) {
    (void) old_buffer;
    (void) memory;
    (void) size;
    (void) arg;

    model_t *pmodel = (model_t *) user_ptr;
    erogator_refresh(pmodel);
}
