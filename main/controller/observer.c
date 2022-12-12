#include "configuration.h"
#include "gel/data_structures/watcher.h"
#include "peripherals/storage.h"
#include "utils/utils.h"


#define NUM_OBSERVED_VARIABLES (5 + (NUM_PROGRAMS * NUM_EROGATORS) * 4)


static void save_schedule_entry(void *mem, void *args);


static watcher_t watchlist[NUM_OBSERVED_VARIABLES + 1] = {0};


void observer_init(model_t *pmodel) {
    size_t i = 0;

    watchlist[i++] = WATCHER(&pmodel->configuration.language, storage_save_uint8, CONFIGURATION_LANGUAGE_KEY);
    watchlist[i++] = WATCHER(&pmodel->configuration.brightness, storage_save_uint8, CONFIGURATION_BRIGHTNESS_KEY);
    watchlist[i++] = WATCHER(&pmodel->configuration.volume, storage_save_uint8, CONFIGURATION_VOLUME_KEY);
    watchlist[i++] = WATCHER(&pmodel->configuration.erogator_percentages[EROGATOR_1], storage_save_uint8,
                             CONFIGURATION_EROGATION_1_PERCENTAGE_KEY);
    watchlist[i++] = WATCHER(&pmodel->configuration.erogator_percentages[EROGATOR_2], storage_save_uint8,
                             CONFIGURATION_EROGATION_2_PERCENTAGE_KEY);

    for (size_t e = 0; e < NUM_EROGATORS; e++) {
        for (size_t j = 0; j < NUM_PROGRAMS; j++) {
            watchlist[i++] = WATCHER(&pmodel->configuration.schedulers[e].entries[j], save_schedule_entry,
                                     CONFIGURATION_SCHEDULER_ENTRIES_KEYS[e][j]);
            watchlist[i++] = WATCHER(&pmodel->configuration.working_modes[e][j], storage_save_uint8,
                                     CONFIGURATION_SCHEDULER_WM_KEYS[e][j]);
            watchlist[i++] = WATCHER(&pmodel->configuration.active_seconds[e][j], storage_save_uint32,
                                     CONFIGURATION_SCHEDULER_ACTIVE_TIME_KEYS[e][j]);
            watchlist[i++] = WATCHER(&pmodel->configuration.pause_seconds[e][j], storage_save_uint32,
                                     CONFIGURATION_SCHEDULER_PAUSE_TIME_KEYS[e][j]);
        }
    }

    assert(NUM_OBSERVED_VARIABLES == i);
    watchlist[i++] = WATCHER_NULL;

    watcher_list_init(watchlist);
}


void observer_observe(void) {
    watcher_process_changes(watchlist, get_millis());
}


static void save_schedule_entry(void *mem, void *args) {
    uint8_t buffer[SCHEDULER_ENTRY_SERIALIZED_SIZE] = {0};
    scheduler_entry_serialize(buffer, mem);
    storage_save_blob(buffer, sizeof(buffer), args);
}