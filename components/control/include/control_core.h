// Control logic with no RTOS dependencies (SPEC 6). The control task owns one
// ctrl_core_t and is its only writer. Host-tested in test/host.
#pragma once

#include "control_types.h"

typedef struct {
    op_mode_t    mode;
    aux_lights_t manual_state;
    aux_lights_t aux_commanded;
    uint16_t     last_applied_req_seq;
} ctrl_core_t;

// Power-on state: MANUAL, both lights off (SPEC 6.2).
void ctrl_core_init(ctrl_core_t *c);

// Apply one request. Returns false if rejected (light requests in AUTO).
bool ctrl_core_apply(ctrl_core_t *c, const ctrl_req_t *req);

// Target light state for the current mode and vehicle inputs.
aux_lights_t ctrl_core_target(const ctrl_core_t *c, vehicle_lights_t veh);

// Recompute the target; if it differs from aux_commanded, store it and return
// true (caller posts it to aux_cmd_q).
bool ctrl_core_update_commanded(ctrl_core_t *c, vehicle_lights_t veh);

// Temperature flag with hysteresis: set at temp <= on_c, cleared at
// temp >= on_c + hyst_c, unchanged in between. False when stale.
bool ctrl_temp_flag(bool prev, bool stale, int temp_c, int on_c, int hyst_c);
