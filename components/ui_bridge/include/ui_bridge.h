// ui_bridge: owns the ui task, the only task that calls LVGL (SPEC 5).
#pragma once

#include "esp_err.h"

// Start the ui task. It initialises LVGL and the input devices itself.
esp_err_t ui_bridge_start(void);
