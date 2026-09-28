#include "control_core.h"

#include <stddef.h>

static bool lights_equal(aux_lights_t a, aux_lights_t b)
{
    return a.ledbar == b.ledbar && a.fog == b.fog;
}

static bool *manual_light(ctrl_core_t *c, aux_light_id_t light)
{
    return light == AUX_LIGHT_LEDBAR ? &c->manual_state.ledbar : &c->manual_state.fog;
}

static void manual_all_off(ctrl_core_t *c)
{
    c->mode = MODE_MANUAL;
    c->manual_state = (aux_lights_t){ 0 };
}

// Safe-state actions, applied on every tick while the link is down.

// Auto logic cannot run on stale inputs (SPEC 6.2): force MANUAL, all off.
// In MANUAL the timeout is a warning only.
static bool can_lights_lost(ctrl_core_t *c)
{
    if (c->mode != MODE_AUTO) {
        return false;
    }
    manual_all_off(c);
    return true;
}

// Link table (SPEC 4.2 item 4). Timeouts come from ctrl_cfg_t; a NULL action
// means warning only. The aux target is re-sent by aux_link on recovery.
static const struct {
    const char *name;
    bool (*on_down)(ctrl_core_t *c);
} s_links[LINK_COUNT] = {
    [LINK_CAN_LIGHTS] = { "can_lights", can_lights_lost },
    [LINK_AUX]        = { "aux",        NULL },
};

const char *ctrl_link_name(ctrl_link_t link)
{
    return link < LINK_COUNT ? s_links[link].name : "?";
}

void ctrl_core_init(ctrl_core_t *c, const ctrl_cfg_t *cfg)
{
    *c = (ctrl_core_t){ .cfg = *cfg, .mode = MODE_MANUAL };
    for (int i = 0; i < LINK_COUNT; i++) {
        c->link_down[i] = true;
    }
}

bool ctrl_core_apply(ctrl_core_t *c, const ctrl_req_t *req)
{
    switch (req->type) {
    case REQ_SET_MODE:
        if (req->mode == MODE_AUTO && c->link_down[LINK_CAN_LIGHTS]) {
            return false;
        }
        if (req->mode == MODE_MANUAL && c->mode == MODE_AUTO) {
            // Bumpless: lights stay as they are at the switch (SPEC 6.2).
            c->manual_state = c->aux_commanded;
        }
        c->mode = req->mode;
        break;

    case REQ_SET_LIGHT:
        if (c->mode != MODE_MANUAL) {
            return false;
        }
        *manual_light(c, req->light) = req->on;
        break;

    case REQ_TOGGLE_LIGHT:
        if (c->mode != MODE_MANUAL) {
            return false;
        }
        // Resolved against manual_state, never against UI state (SPEC 9.2).
        *manual_light(c, req->light) = !*manual_light(c, req->light);
        break;

    case REQ_MANUAL_ALL_OFF:
        manual_all_off(c);
        break;

    default:
        return false;
    }
    c->last_applied_req_seq = req->seq;
    return true;
}

aux_lights_t ctrl_core_target(const ctrl_core_t *c, vehicle_lights_t veh)
{
    if (c->mode == MODE_AUTO) {
        return (aux_lights_t){ .ledbar = veh.main_beam, .fog = veh.front_fog };
    }
    return c->manual_state;
}

bool ctrl_core_update_commanded(ctrl_core_t *c, vehicle_lights_t veh)
{
    aux_lights_t target = ctrl_core_target(c, veh);
    if (lights_equal(target, c->aux_commanded)) {
        return false;
    }
    c->aux_commanded = target;
    return true;
}

bool ctrl_core_tick(ctrl_core_t *c, const uint32_t last_rx_ms[LINK_COUNT], uint32_t now_ms)
{
    bool changed = false;
    for (int i = 0; i < LINK_COUNT; i++) {
        c->link_down[i] = (uint32_t)(now_ms - last_rx_ms[i]) >= c->cfg.link_timeout_ms[i];
        if (c->link_down[i] && s_links[i].on_down) {
            changed |= s_links[i].on_down(c);
        }
    }

    // Commanded vs ACKed mismatch (SPEC 6.2, 7.2).
    if (lights_equal(c->aux_commanded, c->aux_actual)) {
        c->mismatch_tracking = false;
        c->aux_mismatch = false;
    } else {
        if (!c->mismatch_tracking) {
            c->mismatch_tracking = true;
            c->mismatch_since_ms = now_ms;
        }
        c->aux_mismatch = (uint32_t)(now_ms - c->mismatch_since_ms) >= c->cfg.aux_mismatch_ms;
    }
    return changed;
}

void ctrl_core_aux_acked(ctrl_core_t *c, aux_lights_t acked)
{
    c->aux_actual = acked;
}

bool ctrl_temp_flag(bool prev, bool stale, int temp_c, int on_c, int hyst_c)
{
    if (stale) {
        return false;
    }
    if (temp_c <= on_c) {
        return true;
    }
    if (temp_c >= on_c + hyst_c) {
        return false;
    }
    return prev;
}
