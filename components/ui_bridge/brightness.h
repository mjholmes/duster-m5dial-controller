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

// Day/night display state from the headlight switch (0x5DE). The car clears
// the dipped-beam bit while main beam is on (seen on the vehicle), so main
// beam holds the current state: a night drive stays night through main beam
// and a daytime flash stays day. With no fresh CAN light data, hold as well.
bool brightness_night_step(bool prev_night, bool lights_valid, bool dipped_beam, bool main_beam);

// Percent to backlight level 0..255.
uint8_t brightness_to_level(int pct);
