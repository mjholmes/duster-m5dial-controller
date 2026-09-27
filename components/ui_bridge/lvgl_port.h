// Internal to ui_bridge. ui task only.
#pragma once

#include "lvgl.h"
#include "esp_err.h"
#include "sdkconfig.h"

esp_err_t lvgl_port_init(void);

#if CONFIG_UI_BRINGUP_DIAG
// Time full-screen redraws of the active screen and log the frame rate.
void lvgl_port_benchmark(void);
#endif
