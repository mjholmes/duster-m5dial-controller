#include "aux_sm.h"

#include <stddef.h>
#include <stdint.h>

// The library is single-instance and single-owner, so one state machine.
static aux_sm_t *s_sm;

static bool lights_equal(aux_lights_t a, aux_lights_t b)
{
    return a.ledbar == b.ledbar && a.fog == b.fog;
}

// Callbacks run inside auxbeam_light_process(), in the owner task. They only
// record the event; a late callback from an earlier frame is ignored.
static void on_ack(uint8_t seq_id, void *user_data)
{
    (void)seq_id;
    if (s_sm && s_sm->state == AUX_SM_IN_FLIGHT && (uint32_t)(uintptr_t)user_data == s_sm->token) {
        s_sm->evt_ack = true;
    }
}

static void on_timeout(uint8_t seq_id, auxbeam_error_t error, void *user_data)
{
    (void)seq_id;
    (void)error;
    if (s_sm && s_sm->state == AUX_SM_IN_FLIGHT && (uint32_t)(uintptr_t)user_data == s_sm->token) {
        s_sm->evt_timeout = true;
    }
}

// Every command is one absolute set_multiple() frame. Only the two mapped
// channels are OFF/ON; all other RA60 outputs are IGNORE and never touched.
static bool send(aux_sm_t *sm, aux_lights_t state, uint8_t attempt, uint32_t now_ms)
{
    auxbeam_output_state_t states[AUXBEAM_NUM_OUTPUTS];
    for (int i = 0; i < AUXBEAM_NUM_OUTPUTS; i++) {
        states[i] = AUXBEAM_OUTPUT_IGNORE;
    }
    states[sm->cfg.ch_ledbar] = state.ledbar ? AUXBEAM_OUTPUT_ON : AUXBEAM_OUTPUT_OFF;
    states[sm->cfg.ch_fog] = state.fog ? AUXBEAM_OUTPUT_ON : AUXBEAM_OUTPUT_OFF;

    sm->token++;
    sm->evt_ack = false;
    sm->evt_timeout = false;
    if (auxbeam_light_set_multiple(sm->ctx, states, on_ack, on_timeout,
                                   (void *)(uintptr_t)sm->token) != AUXBEAM_OK) {
        return false;
    }
    sm->state = AUX_SM_IN_FLIGHT;
    sm->sent = state;
    sm->attempt = attempt;
    sm->last_send_ms = now_ms;
    return true;
}

void aux_sm_init(aux_sm_t *sm, const aux_sm_cfg_t *cfg, auxbeam_context_t *ctx)
{
    *sm = (aux_sm_t){ .cfg = *cfg, .ctx = ctx, .state = AUX_SM_IDLE };
    s_sm = sm;
}

void aux_sm_set_target(aux_sm_t *sm, aux_lights_t target)
{
    sm->target = target;
}

aux_sm_out_t aux_sm_poll(aux_sm_t *sm, uint32_t now_ms)
{
    aux_sm_out_t out = AUX_SM_OUT_NONE;
    auxbeam_light_process(sm->ctx);

    if (sm->state == AUX_SM_IN_FLIGHT) {
        if (sm->evt_ack) {
            sm->acked = sm->sent;
            sm->have_acked = true;
            sm->state = AUX_SM_IDLE;
            out = AUX_SM_OUT_ACKED;
        } else if (sm->evt_timeout) {
            if (sm->attempt == 1) {
                // Retry once with the same frame content (settled rule).
                if (!send(sm, sm->sent, 2, now_ms)) {
                    sm->state = AUX_SM_BACKOFF;
                    sm->backoff_since_ms = now_ms;
                    return AUX_SM_OUT_FAILED;
                }
            } else {
                sm->state = AUX_SM_BACKOFF;
                sm->backoff_since_ms = now_ms;
                return AUX_SM_OUT_FAILED;
            }
        }
    }

    if (sm->state == AUX_SM_BACKOFF) {
        // Wait one keepalive interval, then re-send the current target.
        if ((uint32_t)(now_ms - sm->backoff_since_ms) >= sm->cfg.keepalive_ms) {
            if (!send(sm, sm->target, 1, now_ms)) {
                sm->backoff_since_ms = now_ms;
            }
        }
        return out;
    }

    if (sm->state == AUX_SM_IDLE) {
        if (!sm->have_acked || !lights_equal(sm->target, sm->acked)) {
            // Also runs straight after an ACK, so a target that changed while
            // the previous frame was in flight goes out at once.
            send(sm, sm->target, 1, now_ms);
        } else if ((uint32_t)(now_ms - sm->last_send_ms) >= sm->cfg.keepalive_ms) {
            // Keepalive: the RA60 sends nothing unsolicited, so re-sending the
            // (idempotent) current state is the only liveness check.
            send(sm, sm->acked, 1, now_ms);
        }
    }
    return out;
}
