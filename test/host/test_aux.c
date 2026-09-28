// Host tests for the RA60 send state machine (SPEC 7.2, 11 step 5), run
// against the real auxbeam-light-controller sources and the library's own
// mock UART and mock clock.

#include <stdio.h>
#include <string.h>
#include "aux_sm.h"
#include "light_protocol.h"
#include "mock_hal_uart.h"

static int s_checks, s_failures;

#define CHECK(cond) do { \
    s_checks++; \
    if (!(cond)) { s_failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

#define FRAME_LEN 11
#define ACK_TIMEOUT_MS 200
#define KEEPALIVE_MS 500
#define POLL_MS 10

static const aux_sm_cfg_t k_cfg = { .ch_ledbar = 1, .ch_fog = 2, .keepalive_ms = KEEPALIVE_MS };

static auxbeam_context_t *s_ctx;
static aux_sm_t s_sm;
static uint32_t s_now;
static int s_acked, s_failed;

static void setup(void)
{
    if (s_ctx) {
        auxbeam_light_deinit(s_ctx);
        s_ctx = NULL;
    }
    mock_hal_uart_reset();
    s_now = 1000;
    mock_clock_set_ms(s_now);
    auxbeam_config_t cfg = { .uart_num = 1, .tx_pin = 2, .rx_pin = 1, .default_timeout_ms = ACK_TIMEOUT_MS };
    auxbeam_light_init(&cfg, &s_ctx);
    aux_sm_init(&s_sm, &k_cfg, s_ctx);
    s_acked = s_failed = 0;
}

// One aux loop iteration after advancing the clock.
static void step(uint32_t ms)
{
    s_now += ms;
    mock_clock_set_ms(s_now);
    aux_sm_out_t out = aux_sm_poll(&s_sm, s_now);
    if (out == AUX_SM_OUT_ACKED) {
        s_acked++;
    } else if (out == AUX_SM_OUT_FAILED) {
        s_failed++;
    }
}

static void run(uint32_t ms)
{
    for (uint32_t t = 0; t < ms; t += POLL_MS) {
        step(POLL_MS);
    }
}

static int frames_sent(void)
{
    return (int)(mock_hal_uart_tx_len() / FRAME_LEN);
}

static const uint8_t *last_frame(void)
{
    return mock_hal_uart_tx_data() + mock_hal_uart_tx_len() - FRAME_LEN;
}

// Expected frame for a light state with the same sequence ID as sent.
static bool frame_is(const uint8_t *frame, bool ledbar, bool fog)
{
    auxbeam_output_state_t states[6] = {
        AUXBEAM_OUTPUT_IGNORE, ledbar ? AUXBEAM_OUTPUT_ON : AUXBEAM_OUTPUT_OFF,
        fog ? AUXBEAM_OUTPUT_ON : AUXBEAM_OUTPUT_OFF, AUXBEAM_OUTPUT_IGNORE,
        AUXBEAM_OUTPUT_IGNORE, AUXBEAM_OUTPUT_IGNORE,
    };
    auxbeam_message_t expect;
    light_protocol_encode_command(&expect, frame[0], states);
    return memcmp(&expect, frame, FRAME_LEN) == 0;
}

static void push_ack(uint8_t seq_id)
{
    // cmd_type 0x18, as the real RA60 replies (library L-5).
    uint8_t ack[FRAME_LEN] = { seq_id, 0xFF, 0x18 };
    ack[10] = light_protocol_calculate_checksum(ack, 10);
    mock_hal_uart_push_rx(ack, sizeof(ack));
}

static void ack_last(void)
{
    push_ack(last_frame()[0]);
}

static void test_power_on_sends_all_off_with_unmapped_ignore(void)
{
    setup();
    step(POLL_MS);
    CHECK(frames_sent() == 1);
    CHECK(frame_is(last_frame(), false, false));   // outputs 2 and 3 OFF, all others IGNORE
    CHECK(last_frame()[4] == 0x80 && last_frame()[5] == 0x08 && last_frame()[6] == 0x88);
    CHECK(s_sm.state == AUX_SM_IN_FLIGHT);
    ack_last();
    step(POLL_MS);
    CHECK(s_acked == 1 && s_sm.have_acked && !s_sm.acked.ledbar && !s_sm.acked.fog);
}

static void test_keepalive_only_when_idle_interval_elapsed(void)
{
    setup();
    step(POLL_MS);
    ack_last();
    step(POLL_MS);
    int sent = frames_sent();
    run(KEEPALIVE_MS - 3 * POLL_MS);
    CHECK(frames_sent() == sent);                  // nothing before the interval
    run(3 * POLL_MS);
    CHECK(frames_sent() == sent + 1);              // keepalive re-sends the ACKed state
    CHECK(frame_is(last_frame(), false, false));
}

static void test_new_target_sent_and_acked(void)
{
    setup();
    step(POLL_MS);
    ack_last();
    step(POLL_MS);
    aux_sm_set_target(&s_sm, (aux_lights_t){ .ledbar = true });
    step(POLL_MS);
    CHECK(frame_is(last_frame(), true, false));
    // Literal bytes (library L-6, output 1 in the high nibble): outputs
    // 1..6 = IGNORE, ON, OFF, IGNORE, IGNORE, IGNORE.
    CHECK(last_frame()[4] == 0x81 && last_frame()[5] == 0x08 && last_frame()[6] == 0x88);
    ack_last();
    step(POLL_MS);
    CHECK(s_sm.acked.ledbar && !s_sm.acked.fog && s_acked == 2);
}

static void test_newest_target_wins_while_in_flight(void)
{
    setup();
    step(POLL_MS);                                  // power-on OFF in flight
    aux_sm_set_target(&s_sm, (aux_lights_t){ .ledbar = true });
    step(POLL_MS);
    aux_sm_set_target(&s_sm, (aux_lights_t){ .fog = true });
    step(POLL_MS);
    CHECK(frames_sent() == 1);                      // one in flight only
    ack_last();
    step(POLL_MS);                                  // ACK, then straight to the newest target
    CHECK(frames_sent() == 2);
    CHECK(frame_is(last_frame(), false, true));     // the intermediate LED-bar-only target never went out
}

static void test_single_timeout_then_retry_succeeds(void)
{
    setup();
    step(POLL_MS);
    const uint8_t first_seq = last_frame()[0];
    run(ACK_TIMEOUT_MS);                            // no ACK: first timeout
    CHECK(frames_sent() == 2);                      // retried once
    CHECK(frame_is(last_frame(), false, false));    // same content
    CHECK(last_frame()[0] != first_seq);            // new sequence ID
    CHECK(s_failed == 0);
    ack_last();
    step(POLL_MS);
    CHECK(s_acked == 1 && s_failed == 0);
}

static void test_two_timeouts_fail_then_backoff(void)
{
    setup();
    aux_sm_set_target(&s_sm, (aux_lights_t){ .ledbar = true });
    step(POLL_MS);
    run(2 * ACK_TIMEOUT_MS + POLL_MS);
    CHECK(s_failed == 1 && s_sm.state == AUX_SM_BACKOFF);
    int sent = frames_sent();
    CHECK(sent == 2);
    run(KEEPALIVE_MS - 2 * POLL_MS);
    CHECK(frames_sent() == sent);                   // quiet during back-off
    run(2 * POLL_MS);
    CHECK(frames_sent() == sent + 1);               // then re-sends the current target
    CHECK(frame_is(last_frame(), true, false));
    ack_last();
    step(POLL_MS);
    CHECK(s_acked == 1 && s_sm.acked.ledbar);       // link recovered
}

// SPEC 11 step 5: a stray byte during an in-flight command costs at most one
// timeout, then the retry succeeds (library L-1 resync).
static void test_stray_byte_during_in_flight_command(void)
{
    setup();
    step(POLL_MS);
    const uint8_t stray = 0x5A;
    mock_hal_uart_push_rx(&stray, 1);
    ack_last();
    run(2 * ACK_TIMEOUT_MS + POLL_MS);
    if (s_acked == 0) {
        ack_last();                                 // ACK the retry
        run(3 * POLL_MS);
    }
    CHECK(s_acked == 1);
    CHECK(s_failed == 0);
    CHECK(frames_sent() <= 2);                      // at most one retry
}

static void test_late_ack_for_first_attempt_is_ignored(void)
{
    setup();
    step(POLL_MS);
    const uint8_t first_seq = last_frame()[0];
    run(ACK_TIMEOUT_MS);                            // timed out, retry in flight
    push_ack(first_seq);                            // late ACK for the abandoned frame
    step(POLL_MS);
    CHECK(s_acked == 0 && s_sm.state == AUX_SM_IN_FLIGHT);
    ack_last();
    step(POLL_MS);
    CHECK(s_acked == 1);
}

int main(void)
{
    test_power_on_sends_all_off_with_unmapped_ignore();
    test_keepalive_only_when_idle_interval_elapsed();
    test_new_target_sent_and_acked();
    test_newest_target_wins_while_in_flight();
    test_single_timeout_then_retry_succeeds();
    test_two_timeouts_fail_then_backoff();
    test_stray_byte_during_in_flight_command();
    test_late_ack_for_first_attempt_is_ignored();
    printf("aux: %d/%d checks passed\n", s_checks - s_failures, s_checks);
    return s_failures ? 1 : 0;
}
