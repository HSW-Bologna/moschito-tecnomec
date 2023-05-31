#include <stdint.h>
#include <stddef.h> /* for offsetof */
#include "watcher.h" /* /components/c-watcher */
#include "rtc.h"
#include "model_watcher.h"
#include "model/model.h"
#include "model/model_descriptor.h"
#include "esp_log.h"

static void model_watcher_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg);

/* watcher instance */
static watcher_t watcher;


static const char *TAG = "RTCModelWatcher";


void model_watcher_init(model_t *pmodel) {
    /* note: second argument value is assigned to watcher.user_ptr */
    watcher_init(&watcher, pmodel);

    size_t model_member_idx = 0;
    model_member_t *model_member;

    while ((model_member = model_descriptor_get_member(model_member_idx)) != NULL) {
        /* watch every model member */
        watcher_add_entry(
            &watcher,
            (unsigned char *) pmodel + model_member->offset,
            model_member->size,
            (void *) model_watcher_cb,
            (void *) (intptr_t) model_member_idx /* arg */
        );
        ++model_member_idx;
    }
}

void model_watcher_watch(void) {
    watcher_watch(&watcher, 0);
}

void model_watcher_trigger_member_silently(size_t model_member_idx) {
    const model_member_t *model_member = model_descriptor_get_member(model_member_idx);
    const int16_t entry_index = watcher_get_entry_index(
        &watcher,
        (unsigned char *) watcher.user_ptr + model_member->offset,
        model_member->size
    );

    if (entry_index >= 0) {
        watcher_trigger_entry_silently(&watcher, entry_index);
    }
}

static void model_watcher_cb(void *old_buffer, const void *memory, size_t size, void *user_ptr, void *arg) {
    const size_t model_watched_member_idx = (size_t) arg;
    model_t *pmodel = (model_t *) user_ptr;
    (void) pmodel;

    ESP_LOGI(TAG, "Watched change on model member with idx %zu", model_watched_member_idx);

    uint8_t *old_buffer_ptr = old_buffer;
    const uint8_t *memory_ptr = memory;
    size_t memory_first_diff_idx = 0;
    size_t memory_last_diff_idx = size - 1;

    /* extract only the part effectively changed */
    for (size_t i = 0; i < size; ++i) {
        if (*(old_buffer_ptr++) != *(memory_ptr++)) {
            if (memory_first_diff_idx == 0) {
                memory_first_diff_idx = i;
            }
            memory_last_diff_idx = i;
        }
    }

    const uint8_t *member_data = memory + memory_first_diff_idx;
    const uint32_t member_data_size = memory_last_diff_idx - memory_first_diff_idx + 1;
    const uint32_t member_data_offset = memory_first_diff_idx;
    const uint16_t member_idx = model_watched_member_idx;
    rtc_send_model_update_message(member_data, member_data_size, member_data_offset, member_idx);
}
