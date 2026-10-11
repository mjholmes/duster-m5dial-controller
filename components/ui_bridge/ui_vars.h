// Internal to ui_bridge. ui task only.
#pragma once

#include <stdint.h>
#include "control.h"
#include "tyre_logic.h"

// Take this loop's model snapshot and format display strings.
void ui_vars_refresh(uint32_t now_ms);

const ui_model_t *ui_vars_model(void);

// Severities computed this loop, for ui_styles.c to apply the matching
// named style without recomputing (and risking drift from the displayed
// text). Coupler state is read straight off ui_vars_model()->etc instead --
// it needs no derivation.
tyre_severity_t ui_vars_tyre_pressure_severity(tyre_pos_t pos);
tyre_severity_t ui_vars_wheel_slip_severity(tyre_pos_t pos);
