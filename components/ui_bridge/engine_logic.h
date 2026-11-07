// Engine readouts (fuel, coolant) derived values -- issue #17. Pure logic,
// host-tested, same shape as tyre_logic.c's tyre_pressure_severity().
#pragma once

#include <stdbool.h>
#include <stdint.h>

// On-device feedback: coolant is shown as plain text tinted blue/red rather
// than a filled severity box, so this is a direct 3-way colour class, not
// the generic 4-level tyre_severity_t.
typedef enum {
    COOLANT_COLOR_NORMAL = 0,
    COOLANT_COLOR_COLD,
    COOLANT_COLOR_HOT,
} coolant_color_t;

// NORMAL if stale (nothing to tint -- "--" stays the default colour).
coolant_color_t coolant_color_class(int16_t temp_c, bool stale, int cold_under_c, int hot_over_c);
