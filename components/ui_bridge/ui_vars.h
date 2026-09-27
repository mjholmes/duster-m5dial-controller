// Internal to ui_bridge. ui task only.
#pragma once

#include <stdint.h>
#include "control.h"

// Take this loop's model snapshot and format display strings.
void ui_vars_refresh(uint32_t now_ms);

const ui_model_t *ui_vars_model(void);
