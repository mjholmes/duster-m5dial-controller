// Display brightness set with the encoder. Pure logic, host-tested.
// Local display state, not part of the model: a Phase 2 remote display sets
// its own brightness.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int min_pct;
    int max_pct;
    int step_pct;     // per encoder detent
} brightness_cfg_t;

// Apply encoder detents (positive = clockwise = brighter) and clamp.
int brightness_step(int pct, int32_t detents, const brightness_cfg_t *cfg);

// Level actually shown: the user's setting, lowered by dim_pct points at night
// (never below min_pct).
int brightness_effective(int pct, bool night, int dim_pct, int min_pct);

// Percent to backlight level 0..255.
uint8_t brightness_to_level(int pct);
