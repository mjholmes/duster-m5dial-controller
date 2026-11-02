// Internal to ui_bridge. ui task only.
#pragma once

#include <stdint.h>
#include "control.h"
#include "tyre_logic.h"
#include "tyre_remap_flow.h"

// Take this loop's model snapshot and format display strings.
void ui_vars_refresh(uint32_t now_ms);

const ui_model_t *ui_vars_model(void);

// Severities computed this loop, for ui_styles.c to apply the matching
// named style without recomputing (and risking drift from the displayed
// text). Coupler state is read straight off ui_vars_model()->etc instead --
// it needs no derivation.
tyre_severity_t ui_vars_tyre_pressure_severity(tyre_pos_t pos);
tyre_severity_t ui_vars_wheel_slip_severity(tyre_pos_t pos);
tyre_severity_t ui_vars_coolant_severity(void);

// Issue #17 (Engine screen), consumed once the EEZ screen exists (not yet
// wired to vars.h/screens -- see the commit that adds the Engine screen).
const char *get_var_coolant_temp_text(void);
void set_var_coolant_temp_text(const char *value);
const char *get_var_fuel_level_text(void);
void set_var_fuel_level_text(const char *value);

// Issue #1: Settings + tyre remap flow, same "not yet wired to vars.h"
// status as the Engine getters above until their screens exist.
const char *get_var_settings_brightness_text(void);
void set_var_settings_brightness_text(const char *value);
const char *get_var_remap_step_text(void);
void set_var_remap_step_text(const char *value);
const char *get_var_remap_corner_text(void);
void set_var_remap_corner_text(const char *value);
const char *get_var_remap_psi_text(void);
void set_var_remap_psi_text(const char *value);
const char *get_var_remap_can_corner_text(void);
void set_var_remap_can_corner_text(const char *value);

// Shared tyre-remap-flow state (explicit-state pure logic; this is the one
// instance ui_bridge keeps, matching ui_vars_model()'s "owns the single
// instance" pattern). ui_actions.c starts/confirms it; ui_bridge.c routes
// encoder input to it while the TyreRemap screen is active.
tyre_remap_t *ui_vars_remap_state(void);
