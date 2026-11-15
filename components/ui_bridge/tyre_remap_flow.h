// TPMS wheel-position remap calibration flow (issue #1). Walks through the
// four physical corners (fixed order: FL, FR, RL, RR, matching tyre_pos_t)
// one at a time; at each, the candidate pool is whichever CAN corners
// haven't been assigned to an earlier physical corner yet, so every path
// through the flow ends in a valid one-to-one permutation -- no special-case
// swap logic needed. Pure logic, explicit state (same convention as
// button_fsm.h/brightness.h), host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int     step;            // 0..3: which physical corner (FL,FR,RL,RR order)
    uint8_t assigned[4];     // physical pos -> CAN pos, valid for indices < step
    int     candidate_idx;   // index into the current step's remaining candidates
} tyre_remap_t;

// Resets to step 0, no corners assigned yet.
void tyre_remap_start(tyre_remap_t *r);

// Cycle the current step's candidate: dir > 0 next, dir < 0 previous, among
// CAN corners not yet assigned to an earlier physical corner this pass.
void tyre_remap_rotate(tyre_remap_t *r, int dir);

// The CAN corner (0..3) the current candidate is, for display.
uint8_t tyre_remap_candidate(const tyre_remap_t *r);

// Confirms the current step's candidate and advances. Returns true once
// all four physical corners are assigned, with out_map filled (physical
// pos -> CAN pos) -- the caller commits it; false if more steps remain.
bool tyre_remap_confirm(tyre_remap_t *r, uint8_t out_map[4]);
