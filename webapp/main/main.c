#include <unistd.h> /* for usleep */
#include <emscripten.h>
#include "sdl/sdl.h"
#include "lvgl.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"
#include "controller/gui.h"
#include "esp_log.h"

static void do_loop(void *arg);


static const char *TAG = "Main";


int main(void)
{
    model_t model;

    lv_init();
    sdl_init();

    model_init(&model);
    view_init(&model, sdl_display_flush, sdl_mouse_read);
    controller_init(&model);

    ESP_LOGI(TAG, "Begin main loop");

    // emscripten_set_main_loop_arg(do_loop, (void*) &model, 100, true);
    emscripten_set_main_loop_arg(do_loop, (void*) &model, 0, true);

    return 0;
}

static void do_loop(void *arg)
{
    model_t *pmodel = (model_t *) arg;
    controller_gui_manage(pmodel);
    controller_manage(pmodel);
}
