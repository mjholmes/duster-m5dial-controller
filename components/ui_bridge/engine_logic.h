// Engine readouts (fuel, coolant) derived values -- issue #17. Pure logic,
// host-tested, same shape as tyre_logic.c's tyre_pressure_severity().
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "tyre_logic.h"

// Reuses tyre_severity_t (OK/CAUTION/ALERT/NODATA) -- it's a generic
// four-level severity, not actually tyre-specific despite the name.
// NODATA if stale; both under- and over-temperature are checked, same
// bounds-with-hysteresis-free shape as tyre_pressure_severity().
tyre_severity_t coolant_severity(int16_t temp_c, bool stale,
                                  int caution_under_c, int alert_under_c,
                                  int caution_over_c, int alert_over_c);
