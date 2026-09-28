// Control logic with no RTOS dependencies (SPEC 6). The control task owns one
// ctrl_core_t and is its only writer. Host-tested in test/host.
#pragma once

#include "control_types.h"

// Links whose health control supervises (SPEC 4.2 item 4). Phase 2 adds a
// remote display link as another entry.
typedef enum {
    LINK_CAN_LIGHTS = 0,   // 0x5DE light inputs
    LINK_AUX,              // RA60 ACKs
    LINK_COUNT
} ctrl_link_t;

typedef struct {
    uint32_t link_timeout_ms[LINK_COUNT];
    uint32_t aux_mismatch_ms;   // commanded != ACKed for this long raises aux_mismatch
} ctrl_cfg_t;

typedef struct {
    ctrl_cfg_t   cfg;
    op_mode_t    mode;
    aux_lights_t manual_state;
    aux_lights_t aux_commanded;
    aux_lights_t aux_actual;       // last ACKed by the RA60
    uint16_t     last_applied_req_seq;
    bool         link_down[LINK_COUNT];
    bool         aux_mismatch;
    bool         mismatch_tracking;
    uint32_t     mismatch_since_ms;
} ctrl_core_t;

// Power-on state: MANUAL, both lights off, every link down until data arrives.
void ctrl_core_init(ctrl_core_t *c, const ctrl_cfg_t *cfg);

// Apply one request. Returns false if rejected: light requests in AUTO, and
// AUTO while the CAN light inputs are lost.
bool ctrl_core_apply(ctrl_core_t *c, const ctrl_req_t *req);

// Target light state for the current mode and vehicle inputs.
aux_lights_t ctrl_core_target(const ctrl_core_t *c, vehicle_lights_t veh);

// Recompute the target; if it differs from aux_commanded, store it and return
// true (caller posts it to aux_cmd_q).
bool ctrl_core_update_commanded(ctrl_core_t *c, vehicle_lights_t veh);

// Periodic supervision. last_rx_ms[i] is the last time link i delivered valid
// data. Updates link_down[], applies each link's safe-state action while it is
// down, and tracks aux_mismatch. Returns true if a safe-state action changed
// mode or manual state (caller then calls ctrl_core_update_commanded()).
bool ctrl_core_tick(ctrl_core_t *c, const uint32_t last_rx_ms[LINK_COUNT], uint32_t now_ms);

// The RA60 acknowledged this absolute state.
void ctrl_core_aux_acked(ctrl_core_t *c, aux_lights_t acked);

const char *ctrl_link_name(ctrl_link_t link);

// Temperature flag with hysteresis: set at temp <= on_c, cleared at
// temp >= on_c + hyst_c, unchanged in between. False when stale.
bool ctrl_temp_flag(bool prev, bool stale, int temp_c, int on_c, int hyst_c);
