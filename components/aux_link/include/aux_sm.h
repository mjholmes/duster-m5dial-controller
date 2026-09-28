// RA60 send state machine (SPEC 7.2) on top of auxbeam-light-controller.
// No RTOS calls: time is passed in, and the library is driven through its
// HAL, so this runs off-target against the library's mock UART.
//
// Must be used only from the library's owner task (the aux task).
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "auxbeam_light.h"
#include "control_types.h"

typedef struct {
    uint8_t  ch_ledbar;        // library output_index (0-based)
    uint8_t  ch_fog;
    uint32_t keepalive_ms;     // idle re-send interval, also the back-off after failure
} aux_sm_cfg_t;

typedef enum { AUX_SM_IDLE, AUX_SM_IN_FLIGHT, AUX_SM_BACKOFF } aux_sm_state_t;

typedef enum {
    AUX_SM_OUT_NONE,
    AUX_SM_OUT_ACKED,     // the RA60 acknowledged the state in aux_sm_t.acked
    AUX_SM_OUT_FAILED,    // two timeouts in a row; entering back-off
} aux_sm_out_t;

typedef struct {
    aux_sm_cfg_t       cfg;
    auxbeam_context_t *ctx;
    aux_sm_state_t     state;
    aux_lights_t       target;
    aux_lights_t       acked;
    bool               have_acked;   // false until the first ACK: the power-on OFF must be sent
    aux_lights_t       sent;         // state of the frame in flight
    uint8_t            attempt;      // 1 = first send, 2 = the single retry
    uint32_t           token;        // identifies the frame in flight to the callbacks
    uint32_t           last_send_ms;
    uint32_t           backoff_since_ms;
    // Recorded by the library callbacks, acted on in aux_sm_poll().
    bool               evt_ack;
    bool               evt_timeout;
} aux_sm_t;

// ctx must be initialised by the owner task. The initial target is both
// lights OFF, sent on the first poll (SPEC 7.2 startup).
void aux_sm_init(aux_sm_t *sm, const aux_sm_cfg_t *cfg, auxbeam_context_t *ctx);

// Newest target wins.
void aux_sm_set_target(aux_sm_t *sm, aux_lights_t target);

// Call every loop (about 10 ms): runs auxbeam_light_process() and the state
// machine. Returns the event to report to control, if any.
aux_sm_out_t aux_sm_poll(aux_sm_t *sm, uint32_t now_ms);
