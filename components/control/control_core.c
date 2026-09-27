#include "control_core.h"

static bool lights_equal(aux_lights_t a, aux_lights_t b)
{
    return a.ledbar == b.ledbar && a.fog == b.fog;
}

static bool *manual_light(ctrl_core_t *c, aux_light_id_t light)
{
    return light == AUX_LIGHT_LEDBAR ? &c->manual_state.ledbar : &c->manual_state.fog;
}

void ctrl_core_init(ctrl_core_t *c)
{
    *c = (ctrl_core_t){ .mode = MODE_MANUAL };
}

bool ctrl_core_apply(ctrl_core_t *c, const ctrl_req_t *req)
{
    switch (req->type) {
    case REQ_SET_MODE:
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
        c->mode = MODE_MANUAL;
        c->manual_state = (aux_lights_t){ 0 };
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
