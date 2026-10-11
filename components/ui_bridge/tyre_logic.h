// Tyre pressure / wheel slip / tyre-warning derived values
// (docs/ui-tyres-4wd-spec.md 5, 9). Pure logic, host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "control_types.h"

typedef enum { TYRE_SEV_NODATA = 0, TYRE_SEV_OK, TYRE_SEV_CAUTION, TYRE_SEV_ALERT } tyre_severity_t;

// raw x 30 mbar -> psi, rounded. Caller checks 0xFF (no value) separately.
int tyre_psi(uint8_t raw);

// NODATA if stale or raw is 0xFF; else by psi against the two thresholds.
// No hysteresis -- the spec applies hysteresis only to wheel slip (below).
tyre_severity_t tyre_pressure_severity(uint8_t raw, bool stale, int caution_psi, int alert_psi);

typedef struct {
    int caution_psi_front;
    int alert_psi_front;
    int caution_psi_rear;
    int alert_psi_rear;
} tyre_psi_cfg_t;

// The instantaneous (pre-debounce) Speedo tyre-warning condition: any wheel's
// pressure severity is CAUTION/ALERT, or its own TPMS wheel state is 5
// (flat) or 6 (under-inflated), or TpmsFailureWarningRequest is set (OQ-29).
// Stale or no-value never raises it (section 9).
bool tyre_warning_condition(const tyre_pressures_t *tp, const tyre_psi_cfg_t *cfg);

// Debounced output: raises after `condition_now` has held true for set_ms,
// clears after it has held false for clear_ms. Caller owns the state
// (static per the usual ui_bridge pattern -- see button_fsm_t).
typedef struct {
    bool     raised;
    bool     condition_prev;
    uint32_t transition_ms;
} tyre_warn_state_t;

bool tyre_warn_update(tyre_warn_state_t *st, bool condition_now, uint32_t now_ms, uint32_t set_ms, uint32_t clear_ms);

// True only when wheel slip is meaningful: not stale and above the speed gate.
bool wheel_slip_gate_open(const wheel_speeds_t *ws, uint16_t speed_gate_centi_kmh);

// Reference speed: mean of the middle two of the four raw wheel values
// after sorting (robust to one wheel being the outlier, e.g. cornering).
uint16_t wheel_speed_median(const uint16_t raw[TYRE_COUNT]);

// (raw - reference) * 100 / reference, clamped to -99..99.
int wheel_slip_pct(uint16_t raw, uint16_t reference);

// Severity on the absolute slip percentage, with hysteresis_pct applied on
// the falling edge only (rising edges always use the plain threshold).
tyre_severity_t wheel_slip_severity(tyre_severity_t prev, bool nodata, int slip_pct_abs,
                                     int caution_pct, int alert_pct, int hysteresis_pct);
