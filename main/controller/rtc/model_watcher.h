#ifndef MODEL_WATCHER_H_INCLUDED
#define MODEL_WATCHER_H_INCLUDED

#include "model/model.h"

void model_watcher_init(model_t *pmodel);
void model_watcher_watch(void);
void model_watcher_trigger_member_silently(size_t model_member_idx);

#endif
