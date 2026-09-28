// OBD client sequencing (SPEC 8.1): ignition-gated start, PID 01 poll, Mode 03
// read over ISO-TP, retry and periodic re-poll. Pure C, time passed in; frames
// to send go into an outbox the caller drains. Host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "control_types.h"
#include "isotp_rx.h"

#define OBD_ID_FUNCTIONAL   0x7DF
#define OBD_ID_ENGINE_REQ   0x7E0   // physical request, and Flow Control target
#define OBD_ID_ENGINE_RESP  0x7E8

typedef struct {
    uint32_t ignition_wait_ms;   // 5000
    uint32_t retry_ms;           // 2000
    uint8_t  retry_count;        // 5 attempts, then DTC_READ_FAILED
    uint32_t repoll_ms;          // 30000
    uint32_t p2_ms;              // 50: response timeout
    uint32_t cf_timeout_ms;      // 150: consecutive frame gap
} obd_cfg_t;

typedef struct {
    uint32_t id;
    uint8_t  data[8];
} obd_frame_t;

typedef enum {
    OBD_WAIT_IGNITION,   // startup: until Pos2/Running or the wait expires
    OBD_WAIT_RUNNING,    // aborted by cranking: until Running
    OBD_PID01_WAIT,      // request sent, waiting for 41 01
    OBD_MODE03_WAIT,     // request sent, waiting for 43 (single or multi-frame)
    OBD_RETRY_WAIT,      // no answer: waiting to retry
    OBD_IDLE,            // waiting for the next periodic poll
} obd_state_t;

typedef struct {
    obd_cfg_t   cfg;
    obd_state_t state;
    uint32_t    start_ms;
    uint32_t    sent_ms;          // last request sent
    uint32_t    next_ms;          // next retry or poll
    uint8_t     attempts;         // failed PID 01 attempts in this round
    uint32_t    req_id;           // ID that last got an answer (Mode 03 uses it)
    bool        codes_valid;      // codes match the current count
    isotp_rx_t  isotp;
    dtc_state_t dtc;
    bool        dtc_changed;      // set when dtc changes; caller clears after publishing
    obd_frame_t outbox[2];
    uint8_t     outbox_len;
} obd_core_t;

void obd_core_init(obd_core_t *c, const obd_cfg_t *cfg, uint32_t now_ms);

// Run once per loop and for every received 0x7E8 frame (rx NULL when none).
// ignition_raw is 0x350 byte 0 (0 = not seen). Afterwards send c->outbox[0..
// outbox_len) in order, then set outbox_len = 0.
void obd_core_step(obd_core_t *c, uint32_t now_ms, uint8_t ignition_raw, const obd_frame_t *rx);
