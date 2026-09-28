#include "obd_core.h"
#include "obd_decode.h"

#include <string.h>

#define IGN_POS2    0xC5
#define IGN_START   0xC6   // cranking: abort, supply dips make the window unreliable
#define IGN_RUNNING 0xC7

static bool elapsed(uint32_t now, uint32_t since, uint32_t ms)
{
    return (uint32_t)(now - since) >= ms;
}

static void push(obd_core_t *c, uint32_t id, const uint8_t *payload, uint8_t n)
{
    if (c->outbox_len >= 2) {
        return;
    }
    obd_frame_t *f = &c->outbox[c->outbox_len++];
    f->id = id;
    memset(f->data, 0x00, sizeof(f->data));   // pad to DLC 8
    memcpy(f->data, payload, n);
}

static void set_dtc(obd_core_t *c, const dtc_state_t *d)
{
    if (memcmp(&c->dtc, d, sizeof(*d)) != 0) {
        c->dtc = *d;
        c->dtc_changed = true;
    }
}

static void set_status(obd_core_t *c, dtc_status_t status)
{
    dtc_state_t d = c->dtc;
    d.status = status;
    set_dtc(c, &d);
}

// Retries alternate functional and physical addressing.
static uint32_t request_id(const obd_core_t *c)
{
    return (c->attempts % 2 == 0) ? OBD_ID_FUNCTIONAL : OBD_ID_ENGINE_REQ;
}

static void send_pid01(obd_core_t *c, uint32_t now)
{
    const uint8_t req[] = { 0x02, 0x01, 0x01 };
    uint32_t id = request_id(c);
    isotp_rx_reset(&c->isotp);
    push(c, id, req, sizeof(req));
    c->req_id = id;
    c->sent_ms = now;
    c->state = OBD_PID01_WAIT;
    if (c->dtc.status == DTC_NOT_READ) {
        set_status(c, DTC_READING);
    }
}

static void send_mode03(obd_core_t *c, uint32_t now)
{
    const uint8_t req[] = { 0x01, 0x03 };
    push(c, c->req_id, req, sizeof(req));
    isotp_rx_reset(&c->isotp);
    c->sent_ms = now;
    c->state = OBD_MODE03_WAIT;
}

static void go_idle(obd_core_t *c, uint32_t now)
{
    c->state = OBD_IDLE;
    c->next_ms = now + c->cfg.repoll_ms;
}

static void start_round(obd_core_t *c, uint32_t now)
{
    c->attempts = 0;
    send_pid01(c, now);
}

static void pid01_failed(obd_core_t *c, uint32_t now)
{
    c->attempts++;
    if (c->attempts >= c->cfg.retry_count) {
        // The periodic re-poll carries on, so a read that failed with the
        // engine off succeeds once it is running (SPEC 8.1 step 6).
        set_status(c, DTC_READ_FAILED);
        go_idle(c, now);
        return;
    }
    c->state = OBD_RETRY_WAIT;
    c->next_ms = c->sent_ms + c->cfg.retry_ms;
}

static void pid01_answered(obd_core_t *c, uint32_t now, bool mil_on, uint8_t count)
{
    dtc_state_t d = c->dtc;
    bool count_changed = d.count != count || d.status == DTC_READING || d.status == DTC_READ_FAILED;
    d.mil_on = mil_on;
    d.count = count;
    d.status = mil_on ? DTC_PRESENT_MIL : count > 0 ? DTC_PRESENT_NO_MIL : DTC_NONE;
    if (count == 0) {
        d.codes_read = 0;
        memset(d.codes, 0, sizeof(d.codes));
        c->codes_valid = true;
    } else if (count_changed) {
        c->codes_valid = false;
    }
    set_dtc(c, &d);

    // Mode 03 only when the count changed, or the last read did not complete.
    if (count > 0 && !c->codes_valid) {
        send_mode03(c, now);
    } else {
        go_idle(c, now);
    }
}

static void mode03_done(obd_core_t *c, uint32_t now)
{
    dtc_state_t d = c->dtc;
    char codes[8][6];
    int n = obd_decode_mode03(c->isotp.buf, c->isotp.len, codes, 8);
    if (n >= 0) {
        memset(d.codes, 0, sizeof(d.codes));
        memcpy(d.codes, codes, (size_t)n * sizeof(codes[0]));
        d.codes_read = (uint8_t)n;
        c->codes_valid = true;
        set_dtc(c, &d);
    }
    go_idle(c, now);
}

void obd_core_init(obd_core_t *c, const obd_cfg_t *cfg, uint32_t now_ms)
{
    memset(c, 0, sizeof(*c));
    c->cfg = *cfg;
    c->state = OBD_WAIT_IGNITION;
    c->start_ms = now_ms;
    c->req_id = OBD_ID_FUNCTIONAL;
    c->dtc.status = DTC_NOT_READ;
}

void obd_core_step(obd_core_t *c, uint32_t now, uint8_t ign, const obd_frame_t *rx)
{
    // Cranking aborts any transaction (and the startup wait); restart once
    // Running is seen.
    if (ign == IGN_START && c->state != OBD_WAIT_RUNNING) {
        isotp_rx_reset(&c->isotp);
        c->state = OBD_WAIT_RUNNING;
    }

    switch (c->state) {
    case OBD_WAIT_IGNITION:
        if (ign == IGN_POS2 || ign == IGN_RUNNING || elapsed(now, c->start_ms, c->cfg.ignition_wait_ms)) {
            start_round(c, now);
        }
        break;

    case OBD_WAIT_RUNNING:
        if (ign == IGN_RUNNING) {
            start_round(c, now);
        }
        break;

    case OBD_PID01_WAIT:
        if (rx && rx->id == OBD_ID_ENGINE_RESP) {
            if (isotp_rx_feed(&c->isotp, rx->data, 8, now) == ISOTP_RX_DONE) {
                bool mil;
                uint8_t count;
                if (obd_decode_pid01(c->isotp.buf, c->isotp.len, &mil, &count)) {
                    pid01_answered(c, now, mil, count);
                    break;
                }
            }
        }
        if (elapsed(now, c->sent_ms, c->cfg.p2_ms)) {
            pid01_failed(c, now);
        }
        break;

    case OBD_MODE03_WAIT:
        if (rx && rx->id == OBD_ID_ENGINE_RESP) {
            isotp_rx_result_t r = isotp_rx_feed(&c->isotp, rx->data, 8, now);
            if (r == ISOTP_RX_SEND_FC) {
                // Flow Control goes to the physical ID, also after a
                // functional request (SPEC 8.2).
                uint8_t fc[8];
                isotp_flow_control(fc);
                push(c, OBD_ID_ENGINE_REQ, fc, 8);
                break;
            }
            if (r == ISOTP_RX_DONE) {
                mode03_done(c, now);
                break;
            }
            if (r == ISOTP_RX_ERROR) {
                go_idle(c, now);   // codes_valid stays false: re-read at the next poll
                break;
            }
        }
        if (c->isotp.active ? isotp_rx_check_timeout(&c->isotp, now, c->cfg.cf_timeout_ms)
                            : elapsed(now, c->sent_ms, c->cfg.p2_ms)) {
            go_idle(c, now);
        }
        break;

    case OBD_RETRY_WAIT:
        if ((int32_t)(now - c->next_ms) >= 0) {
            send_pid01(c, now);
        }
        break;

    case OBD_IDLE:
        if ((int32_t)(now - c->next_ms) >= 0) {
            start_round(c, now);
        }
        break;
    }
}
