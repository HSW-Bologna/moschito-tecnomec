#include <stddef.h> /* for offsetof, typeof */
#include "model.h"
#include "model_descriptor.h"
#include "gel/scheduler/scheduler.h"

// https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
#define member_size(type, member) sizeof(((type*)0)->member)

/* array map : idx -> scalar model member */
static model_member_t *model_members;
static size_t model_members_len;

model_member_t* model_descriptor_get_member(size_t model_member_idx) {
    return model_member_idx < model_members_len ? &model_members[model_member_idx] : NULL;
}

void model_descriptor_init() {
    /* --- description of the model struct --- */

    /* note: only scalar members are considered
     * the idea is to consider nested struct members separately
     * and then join them at the end in the model_members array */

    /* model configuration */
    const model_member_t MODEL_CONFIGURATION = {
        offsetof(model_t, configuration),
        member_size(model_t, configuration)
    };
    const model_member_t MODEL_CONFIGURATION_SCHEMA[] = {
#define MODEL_CONFIGURATION_TYPE typeof(((model_t*)0)->configuration)
        {
            offsetof(MODEL_CONFIGURATION_TYPE, language),
            member_size(MODEL_CONFIGURATION_TYPE, language)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, erogation_seconds),
            member_size(MODEL_CONFIGURATION_TYPE, erogation_seconds)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, brightness),
            member_size(MODEL_CONFIGURATION_TYPE, brightness)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, volume),
            member_size(MODEL_CONFIGURATION_TYPE, volume)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, erogator_percentages),
            member_size(MODEL_CONFIGURATION_TYPE, erogator_percentages)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, working_modes),
            member_size(MODEL_CONFIGURATION_TYPE, working_modes)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, active_seconds),
            member_size(MODEL_CONFIGURATION_TYPE, active_seconds)
        },
        {
            offsetof(MODEL_CONFIGURATION_TYPE, pause_seconds),
            member_size(MODEL_CONFIGURATION_TYPE, pause_seconds)
        }
#undef MODEL_CONFIGURATION_TYPE
    };

    /* model run */
    const model_member_t MODEL_RUN = {
        offsetof(model_t, run),
        member_size(model_t, run)
    };
    const model_member_t MODEL_RUN_SCHEMA[] = {
#define MODEL_RUN_TYPE typeof(((model_t*)0)->run)
        {
            offsetof(MODEL_RUN_TYPE, stop),
            member_size(MODEL_RUN_TYPE, stop)
        },
        {
            offsetof(MODEL_RUN_TYPE, stop_time),
            member_size(MODEL_RUN_TYPE, stop_time)
        },
        {
            offsetof(MODEL_RUN_TYPE, erogators_state),
            member_size(MODEL_RUN_TYPE, erogators_state)
        },
        {
            offsetof(MODEL_RUN_TYPE, missing_water_alarm),
            member_size(MODEL_RUN_TYPE, missing_water_alarm)
        },
        {
            offsetof(MODEL_RUN_TYPE, missing_product),
            member_size(MODEL_RUN_TYPE, missing_product)
        }
#undef MODEL_RUN_TYPE
    };

    /* model schedulers */
    /* note: model schedulers is an array of scheduler structs
     * only the first element is considered,
     * its all we need to have a description of the alignment of the model struct  */
    const model_member_t MODEL_SCHEDULERS = {
        offsetof(model_t, configuration.schedulers[0]),
        member_size(model_t, configuration.schedulers[0])
    };
    const model_member_t MODEL_SCHEDULERS_SCHEMA[] = {
#define MODEL_SCHEDULERS_TYPE typeof(((model_t*)0)->configuration.schedulers[0])
#ifdef GEL_SCHEDULER_DYNAMIC_ENTRIES
        {
            offsetof(MODEL_SCHEDULERS_TYPE, num_entries),
            member_size(MODEL_SCHEDULERS_TYPE, num_entries)
        },
        {
            offsetof(MODEL_SCHEDULERS_TYPE, entries),
            member_size(MODEL_SCHEDULERS_TYPE, entries)
        }
#else
        {} /* no members */
#endif
#undef MODEL_SCHEDULERS_TYPE
    };

    /* model scheduler entries */
    const model_member_t MODEL_SCHEDULER_ENTRIES = {
        offsetof(model_t, configuration.schedulers[0].entries[0]),
        member_size(model_t, configuration.schedulers[0].entries[0])
    };
    const model_member_t MODEL_SCHEDULER_ENTRIES_SCHEMA[] = {
#define MODEL_SCHEDULER_ENTRIES_TYPE typeof(((model_t*)0)->configuration.schedulers[0].entries[0])
#ifdef GEL_SCHEDULER_DYNAMIC_ENTRIES
        {} /* no members */
#else
        {
            offsetof(MODEL_SCHEDULER_ENTRIES_TYPE, enabled),
            member_size(MODEL_SCHEDULER_ENTRIES_TYPE, enabled)
        },
        {
            offsetof(MODEL_SCHEDULER_ENTRIES_TYPE, days),
            member_size(MODEL_SCHEDULER_ENTRIES_TYPE, days)
        },
        {
            offsetof(MODEL_SCHEDULER_ENTRIES_TYPE, start_second),
            member_size(MODEL_SCHEDULER_ENTRIES_TYPE, start_second)
        },
        {
            offsetof(MODEL_SCHEDULER_ENTRIES_TYPE, end_second),
            member_size(MODEL_SCHEDULER_ENTRIES_TYPE, end_second)
        },
        {
            offsetof(MODEL_SCHEDULER_ENTRIES_TYPE, value),
            member_size(MODEL_SCHEDULER_ENTRIES_TYPE, value)
        }
#endif
#undef MODEL_SCHEDULER_ENTRIES_TYPE
    };

    /* the enum is used since the dimension of an array needs to be an integer constant */
    enum {
        MODEL_CONFIGURATION_SCHEMA_LEN = (sizeof MODEL_CONFIGURATION_SCHEMA / sizeof(model_member_t)),
        MODEL_RUN_SCHEMA_LEN = (sizeof MODEL_RUN_SCHEMA / sizeof(model_member_t)),
#ifdef GEL_SCHEDULER_DYNAMIC_ENTRIES
        MODEL_SCHEDULERS_SCHEMA_LEN = (sizeof MODEL_SCHEDULERS_SCHEMA / sizeof(model_member_t)),
        MODEL_SCHEDULER_ENTRIES_SCHEMA_LEN = 0,
#else
        MODEL_SCHEDULERS_SCHEMA_LEN = 0,
        MODEL_SCHEDULER_ENTRIES_SCHEMA_LEN = (sizeof MODEL_SCHEDULER_ENTRIES_SCHEMA / sizeof(model_member_t)),
#endif
    };

    /* --- model members array --- */

    /* model_members lists the members of the model struct flatly
     * this is done for hiding the complexity of the model
     * especially since there are members that are array of structs (e.g. schedulers) */
#define MODEL_MEMBERS_SIZE\
    MODEL_CONFIGURATION_SCHEMA_LEN +\
    MODEL_RUN_SCHEMA_LEN +\
    MODEL_SCHEDULERS_SCHEMA_LEN * NUM_EROGATORS +\
    MODEL_SCHEDULER_ENTRIES_SCHEMA_LEN * NUM_EROGATORS * GEL_SCHEDULER_MAX_ENTRIES

    static model_member_t model_members_hidden[MODEL_MEMBERS_SIZE];
    /* init the alias with file scope */
    model_members = model_members_hidden;
    model_members_len = MODEL_MEMBERS_SIZE;
#undef MODEL_MEMBERS_SIZE

    size_t i, j, k, model_members_idx = 0;

    /* model configuration */
    for (i = 0; i < MODEL_CONFIGURATION_SCHEMA_LEN; ++i) {
        model_members[model_members_idx++] = (model_member_t) {
            MODEL_CONFIGURATION.offset + MODEL_CONFIGURATION_SCHEMA[i].offset,
            MODEL_CONFIGURATION_SCHEMA[i].size
        };
    }

    /* model run */
    for (i = 0; i < MODEL_RUN_SCHEMA_LEN; ++i) {
        model_members[model_members_idx++] = (model_member_t) {
            MODEL_RUN.offset + MODEL_RUN_SCHEMA[i].offset,
            MODEL_RUN_SCHEMA[i].size
        };
    }

    /* model schedulers */
    for (i = 0; i < MODEL_SCHEDULERS_SCHEMA_LEN; ++i) {
        for (j = 0; j < NUM_EROGATORS; ++j) {
            model_members[model_members_idx++] = (model_member_t) {
                MODEL_SCHEDULERS.offset + j*MODEL_SCHEDULERS.size + MODEL_SCHEDULERS_SCHEMA[i].offset,
                MODEL_SCHEDULERS_SCHEMA[i].size
            };
        }
    }

    /* model scheduler entries */
    for (i = 0; i < MODEL_SCHEDULER_ENTRIES_SCHEMA_LEN; ++i) {
        for (j = 0; j < NUM_EROGATORS; ++j) {
            for (k = 0; k < GEL_SCHEDULER_MAX_ENTRIES; ++k) {
                model_members[model_members_idx++] = (model_member_t) {
                    MODEL_SCHEDULER_ENTRIES.offset + j*MODEL_SCHEDULERS.size + k*MODEL_SCHEDULER_ENTRIES.size + MODEL_SCHEDULER_ENTRIES_SCHEMA[i].offset,
                    MODEL_SCHEDULER_ENTRIES_SCHEMA[i].size
                };
            }
        }
    }
}
