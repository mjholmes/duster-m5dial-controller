// Host tests for the OBD client (SPEC 8): ISO-TP receive, PID 01 / Mode 03
// decoding, and the request sequencing against a simulated engine ECU.

#include <stdio.h>
#include <string.h>
#include "isotp_rx.h"
#include "obd_decode.h"
#include "obd_core.h"

extern int g_obd_checks, g_obd_failures;
int g_obd_checks, g_obd_failures;

#define CHECK(cond) do { \
    g_obd_checks++; \
    if (!(cond)) { g_obd_failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

// ---------------------------------------------------------------- ISO-TP

static void test_isotp_single_frame(void)
{
    isotp_rx_t rx;
    isotp_rx_reset(&rx);
    const uint8_t sf[8] = { 0x06, 0x41, 0x01, 0x83, 0x07, 0xE5, 0x00, 0xAA };
    CHECK(isotp_rx_feed(&rx, sf, 8, 0) == ISOTP_RX_DONE);
    CHECK(rx.len == 6 && rx.buf[0] == 0x41 && rx.buf[5] == 0x00);
}

static void test_isotp_multi_frame(void)
{
    isotp_rx_t rx;
    isotp_rx_reset(&rx);
    // 18-byte message: FF carries 6, then CFs 1 and 2 carry 7 and 5.
    const uint8_t ff[8]  = { 0x10, 0x12, 0x43, 0x08, 0x01, 0x01, 0x02, 0x02 };
    const uint8_t cf1[8] = { 0x21, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06 };
    const uint8_t cf2[8] = { 0x22, 0x06, 0x07, 0x07, 0x08, 0x08, 0xAA, 0xAA };
    CHECK(isotp_rx_feed(&rx, ff, 8, 0) == ISOTP_RX_SEND_FC);
    CHECK(isotp_rx_feed(&rx, cf1, 8, 10) == ISOTP_RX_NONE);
    CHECK(isotp_rx_feed(&rx, cf2, 8, 20) == ISOTP_RX_DONE);
    CHECK(rx.len == 18 && rx.buf[0] == 0x43 && rx.buf[17] == 0x08);
}

static void test_isotp_sequence_gap_aborts(void)
{
    isotp_rx_t rx;
    isotp_rx_reset(&rx);
    const uint8_t ff[8]  = { 0x10, 0x12, 0, 0, 0, 0, 0, 0 };
    const uint8_t cf2[8] = { 0x22, 0, 0, 0, 0, 0, 0, 0 };   // CF 1 missing
    isotp_rx_feed(&rx, ff, 8, 0);
    CHECK(isotp_rx_feed(&rx, cf2, 8, 10) == ISOTP_RX_ERROR);
    CHECK(!rx.active);
}

static void test_isotp_sequence_wraps_f_to_0(void)
{
    isotp_rx_t rx;
    isotp_rx_reset(&rx);
    // 6 + 16 * 7 = 118 bytes: CF sequence numbers 1..F then 0.
    const uint8_t ff[8] = { 0x10, 118, 0, 0, 0, 0, 0, 0 };
    isotp_rx_feed(&rx, ff, 8, 0);
    isotp_rx_result_t r = ISOTP_RX_NONE;
    for (int i = 1; i <= 16; i++) {
        uint8_t cf[8] = { (uint8_t)(0x20 | (i & 0x0F)), 0, 0, 0, 0, 0, 0, 0 };
        r = isotp_rx_feed(&rx, cf, 8, (uint32_t)i);
    }
    CHECK(r == ISOTP_RX_DONE);
    CHECK(rx.len == ISOTP_RX_MAX);   // truncated to the buffer, message still completed
}

static void test_isotp_cf_timeout(void)
{
    isotp_rx_t rx;
    isotp_rx_reset(&rx);
    const uint8_t ff[8] = { 0x10, 0x12, 0, 0, 0, 0, 0, 0 };
    isotp_rx_feed(&rx, ff, 8, 1000);
    CHECK(!isotp_rx_check_timeout(&rx, 1149, 150));
    CHECK(isotp_rx_check_timeout(&rx, 1150, 150));
    CHECK(!rx.active);
}

static void test_isotp_flow_control_frame(void)
{
    uint8_t fc[8];
    isotp_flow_control(fc);
    const uint8_t expect[8] = { 0x30, 0, 0, 0, 0, 0, 0, 0 };
    CHECK(memcmp(fc, expect, 8) == 0);
}

// ---------------------------------------------------------------- decoding

static void test_decode_pid01(void)
{
    const uint8_t p[] = { 0x41, 0x01, 0x83, 0x07, 0xE5, 0x00 };
    bool mil;
    uint8_t n;
    CHECK(obd_decode_pid01(p, sizeof(p), &mil, &n) && mil && n == 3);
    const uint8_t q[] = { 0x41, 0x01, 0x02, 0x00, 0x00, 0x00 };
    CHECK(obd_decode_pid01(q, sizeof(q), &mil, &n) && !mil && n == 2);
    const uint8_t neg[] = { 0x7F, 0x01, 0x12 };
    CHECK(!obd_decode_pid01(neg, sizeof(neg), &mil, &n));
}

static void test_dtc_string(void)
{
    char s[6];
    obd_dtc_to_string(0x03, 0x01, s);
    CHECK(strcmp(s, "P0301") == 0);    // SPEC 8.3 example
    obd_dtc_to_string(0x04, 0x20, s);
    CHECK(strcmp(s, "P0420") == 0);
    obd_dtc_to_string(0x41, 0x23, s);
    CHECK(strcmp(s, "C0123") == 0);
    obd_dtc_to_string(0xC1, 0x00, s);
    CHECK(strcmp(s, "U0100") == 0);
    obd_dtc_to_string(0x9A, 0xBC, s);
    CHECK(strcmp(s, "B1ABC") == 0);
}

static void test_decode_mode03_both_layouts(void)
{
    char c[8][6];
    // With count byte: 43 02 03 01 04 20 (even length).
    const uint8_t with[] = { 0x43, 0x02, 0x03, 0x01, 0x04, 0x20 };
    CHECK(obd_decode_mode03(with, sizeof(with), c, 8) == 2);
    CHECK(strcmp(c[0], "P0301") == 0 && strcmp(c[1], "P0420") == 0);
    // Without count byte: 43 03 01 04 20 00 00 (odd length, zero padding skipped).
    const uint8_t without[] = { 0x43, 0x03, 0x01, 0x04, 0x20, 0x00, 0x00 };
    CHECK(obd_decode_mode03(without, sizeof(without), c, 8) == 2);
    CHECK(strcmp(c[0], "P0301") == 0 && strcmp(c[1], "P0420") == 0);
    // No codes.
    const uint8_t none[] = { 0x43, 0x00 };
    CHECK(obd_decode_mode03(none, sizeof(none), c, 8) == 0);
    // Cap at max_codes.
    CHECK(obd_decode_mode03(with, sizeof(with), c, 1) == 1);
    // Not a Mode 03 response.
    const uint8_t neg[] = { 0x7F, 0x03, 0x12 };
    CHECK(obd_decode_mode03(neg, sizeof(neg), c, 8) == -1);
}

// ---------------------------------------------------------------- sequencing

static const obd_cfg_t k_cfg = {
    .ignition_wait_ms = 5000, .retry_ms = 2000, .retry_count = 5,
    .repoll_ms = 30000, .p2_ms = 50, .cf_timeout_ms = 150,
};

static obd_core_t s_c;
static uint32_t s_now;
static obd_frame_t s_sent[16];
static int s_nsent;

static void drain(void)
{
    for (int i = 0; i < s_c.outbox_len && s_nsent < 16; i++) {
        s_sent[s_nsent++] = s_c.outbox[i];
    }
    s_c.outbox_len = 0;
}

static void tick(uint32_t ms, uint8_t ign)
{
    s_now += ms;
    obd_core_step(&s_c, s_now, ign, NULL);
    drain();
}

static void ecu(const uint8_t d[8], uint8_t ign)
{
    s_now += 5;
    obd_frame_t f = { .id = OBD_ID_ENGINE_RESP };
    memcpy(f.data, d, 8);
    obd_core_step(&s_c, s_now, ign, &f);
    drain();
}

static void start(void)
{
    s_now = 1000;
    s_nsent = 0;
    obd_core_init(&s_c, &k_cfg, s_now);
}

static bool sent_is(int i, uint32_t id, const uint8_t *bytes, int n)
{
    if (i >= s_nsent || s_sent[i].id != id || memcmp(s_sent[i].data, bytes, (size_t)n) != 0) {
        return false;
    }
    for (int k = n; k < 8; k++) {
        if (s_sent[i].data[k] != 0x00) {
            return false;   // padded to DLC 8
        }
    }
    return true;
}

static const uint8_t PID01_REQ[] = { 0x02, 0x01, 0x01 };
static const uint8_t MODE03_REQ[] = { 0x01, 0x03 };

static void test_waits_for_ignition_then_polls(void)
{
    start();
    tick(10, 0xC4);                               // Pos1: wait
    CHECK(s_nsent == 0 && s_c.dtc.status == DTC_NOT_READ);
    tick(10, 0xC5);                               // Pos2: go
    CHECK(sent_is(0, OBD_ID_FUNCTIONAL, PID01_REQ, 3));
    CHECK(s_c.dtc.status == DTC_READING);
}

static void test_no_ignition_starts_after_wait(void)
{
    start();
    tick(4990, 0);
    CHECK(s_nsent == 0);
    tick(10, 0);                                  // 5 s: continue anyway
    CHECK(s_nsent == 1);
}

static void test_no_dtcs(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x00, 0x07, 0xE5, 0x00, 0x00 };
    ecu(r, 0xC7);
    CHECK(s_c.dtc.status == DTC_NONE && s_c.dtc.count == 0 && !s_c.dtc.mil_on);
    CHECK(s_nsent == 1);                          // no Mode 03
    CHECK(s_c.state == OBD_IDLE && s_c.dtc_changed);
}

static void test_dtcs_single_frame_mode03(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x02, 0x07, 0xE5, 0x00, 0x00 };  // 2 DTCs, MIL off
    ecu(r, 0xC7);
    CHECK(s_c.dtc.status == DTC_PRESENT_NO_MIL && s_c.dtc.count == 2);
    CHECK(sent_is(1, OBD_ID_FUNCTIONAL, MODE03_REQ, 2));
    const uint8_t m[8] = { 0x06, 0x43, 0x02, 0x03, 0x01, 0x04, 0x20, 0x00 };
    ecu(m, 0xC7);
    CHECK(s_c.dtc.codes_read == 2);
    CHECK(strcmp(s_c.dtc.codes[0], "P0301") == 0 && strcmp(s_c.dtc.codes[1], "P0420") == 0);
    CHECK(s_c.state == OBD_IDLE);
}

static void test_dtcs_multi_frame_mode03_with_flow_control(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x85, 0, 0, 0, 0 };  // 5 DTCs, MIL on
    ecu(r, 0xC7);
    CHECK(s_c.dtc.status == DTC_PRESENT_MIL && s_c.dtc.mil_on && s_c.dtc.count == 5);
    // 43 05 + 5 codes = 12 bytes: FF + one CF.
    const uint8_t ff[8] = { 0x10, 0x0C, 0x43, 0x05, 0x01, 0x01, 0x01, 0x02 };
    ecu(ff, 0xC7);
    CHECK(s_nsent == 3);
    const uint8_t fc[8] = { 0x30, 0, 0, 0, 0, 0, 0, 0 };
    CHECK(sent_is(2, OBD_ID_ENGINE_REQ, fc, 8));  // FC to the physical ID after a functional request
    const uint8_t cf[8] = { 0x21, 0x01, 0x03, 0x01, 0x04, 0x01, 0x05, 0xAA };
    ecu(cf, 0xC7);
    CHECK(s_c.dtc.codes_read == 5);
    CHECK(strcmp(s_c.dtc.codes[0], "P0101") == 0 && strcmp(s_c.dtc.codes[4], "P0105") == 0);
}

static void test_retry_alternates_ids_then_fails_and_repolls(void)
{
    start();
    tick(10, 0xC7);
    CHECK(sent_is(0, OBD_ID_FUNCTIONAL, PID01_REQ, 3));
    tick(50, 0xC7);                               // P2 expired: attempt 1 failed
    CHECK(s_c.state == OBD_RETRY_WAIT && s_nsent == 1);
    tick(1950, 0xC7);                             // 2 s after the first send
    CHECK(s_nsent == 2 && sent_is(1, OBD_ID_ENGINE_REQ, PID01_REQ, 3));  // physical fallback
    for (int i = 0; i < 3; i++) {
        tick(50, 0xC7);
        tick(1950, 0xC7);
    }
    CHECK(s_nsent == 5);                          // 5 attempts
    tick(50, 0xC7);
    CHECK(s_c.dtc.status == DTC_READ_FAILED && s_c.state == OBD_IDLE);

    // The periodic re-poll continues, and succeeds once the ECU answers.
    tick(30000, 0xC7);
    CHECK(s_nsent == 6);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x00, 0, 0, 0, 0 };
    ecu(r, 0xC7);
    CHECK(s_c.dtc.status == DTC_NONE);
}

static void test_mode03_only_when_count_changes(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x01, 0, 0, 0, 0 };
    ecu(r, 0xC7);
    const uint8_t m[8] = { 0x04, 0x43, 0x01, 0x03, 0x01, 0, 0, 0 };
    ecu(m, 0xC7);
    CHECK(s_nsent == 2 && s_c.dtc.codes_read == 1);

    tick(30000, 0xC7);                            // periodic poll
    CHECK(s_nsent == 3);
    ecu(r, 0xC7);                                 // same count
    CHECK(s_nsent == 3 && s_c.state == OBD_IDLE); // no Mode 03

    tick(30000, 0xC7);
    const uint8_t r2[8] = { 0x06, 0x41, 0x01, 0x02, 0, 0, 0, 0 };
    ecu(r2, 0xC7);                                // count changed
    CHECK(s_nsent == 5 && sent_is(4, OBD_ID_FUNCTIONAL, MODE03_REQ, 2));
}

static void test_failed_mode03_rereads_next_poll(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x01, 0, 0, 0, 0 };
    ecu(r, 0xC7);
    tick(50, 0xC7);                               // Mode 03 unanswered
    CHECK(s_c.state == OBD_IDLE && s_c.dtc.count == 1 && s_c.dtc.codes_read == 0);
    tick(30000, 0xC7);
    ecu(r, 0xC7);                                 // same count, codes still missing
    CHECK(s_nsent == 4 && sent_is(3, OBD_ID_FUNCTIONAL, MODE03_REQ, 2));
}

static void test_cranking_aborts_and_restarts_when_running(void)
{
    start();
    tick(10, 0xC5);
    CHECK(s_nsent == 1);
    tick(10, 0xC6);                               // cranking mid-transaction
    CHECK(s_c.state == OBD_WAIT_RUNNING);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x00, 0, 0, 0, 0 };
    ecu(r, 0xC6);                                 // answer during cranking is ignored
    CHECK(s_c.dtc.status == DTC_READING);
    tick(500, 0xC6);
    CHECK(s_nsent == 1);
    tick(10, 0xC7);                               // running: restart
    CHECK(s_nsent == 2 && sent_is(1, OBD_ID_FUNCTIONAL, PID01_REQ, 3));
}

static void test_engine_start_rereads_between_polls(void)
{
    start();
    tick(10, 0xC7);
    const uint8_t r[8] = { 0x06, 0x41, 0x01, 0x00, 0, 0, 0, 0 };
    ecu(r, 0xC7);
    CHECK(s_c.state == OBD_IDLE && s_nsent == 1);
    tick(20000, 0xC3);                            // engine stopped, before the next poll (30 s in k_cfg)
    CHECK(s_nsent == 1);
    tick(10, 0xC6);                               // restart: cranking
    CHECK(s_c.state == OBD_WAIT_RUNNING);
    tick(800, 0xC7);                              // running: fresh read at once
    CHECK(s_nsent == 2 && sent_is(1, OBD_ID_FUNCTIONAL, PID01_REQ, 3));
}

static void test_other_ids_ignored(void)
{
    start();
    tick(10, 0xC7);
    obd_frame_t f = { .id = 0x7E9, .data = { 0x06, 0x41, 0x01, 0x05, 0, 0, 0, 0 } };
    s_now += 5;
    obd_core_step(&s_c, s_now, 0xC7, &f);
    CHECK(s_c.dtc.status == DTC_READING && s_c.state == OBD_PID01_WAIT);
}

void run_obd_tests(void)
{
    test_isotp_single_frame();
    test_isotp_multi_frame();
    test_isotp_sequence_gap_aborts();
    test_isotp_sequence_wraps_f_to_0();
    test_isotp_cf_timeout();
    test_isotp_flow_control_frame();
    test_decode_pid01();
    test_dtc_string();
    test_decode_mode03_both_layouts();
    test_waits_for_ignition_then_polls();
    test_no_ignition_starts_after_wait();
    test_no_dtcs();
    test_dtcs_single_frame_mode03();
    test_dtcs_multi_frame_mode03_with_flow_control();
    test_retry_alternates_ids_then_fails_and_repolls();
    test_mode03_only_when_count_changes();
    test_failed_mode03_rereads_next_poll();
    test_cranking_aborts_and_restarts_when_running();
    test_engine_start_rereads_between_polls();
    test_other_ids_ignored();
}
