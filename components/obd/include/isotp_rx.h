// Minimal ISO-TP receiver (ISO 15765-2), only what Mode 03 needs (SPEC 8.2).
// Pure C, time passed in, host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define ISOTP_RX_MAX 64   // payload bytes kept; longer messages are received and truncated

typedef enum {
    ISOTP_RX_NONE,       // frame consumed, message not complete yet
    ISOTP_RX_SEND_FC,    // first frame received: caller sends Flow Control now
    ISOTP_RX_DONE,       // message complete in rx->buf / rx->len
    ISOTP_RX_ERROR,      // sequence gap or unexpected frame; message discarded
} isotp_rx_result_t;

typedef struct {
    uint8_t  buf[ISOTP_RX_MAX];
    uint16_t len;         // bytes stored (<= ISOTP_RX_MAX)
    uint16_t total;       // length announced by the sender
    uint16_t received;
    uint8_t  next_sn;
    bool     active;      // multi-frame message in progress
    uint32_t last_ms;
} isotp_rx_t;

void isotp_rx_reset(isotp_rx_t *rx);

// Feed one CAN frame payload (DLC bytes) received at now_ms.
isotp_rx_result_t isotp_rx_feed(isotp_rx_t *rx, const uint8_t *data, uint8_t dlc, uint32_t now_ms);

// True if a multi-frame message has waited longer than cf_timeout_ms for its
// next consecutive frame. The message is discarded.
bool isotp_rx_check_timeout(isotp_rx_t *rx, uint32_t now_ms, uint32_t cf_timeout_ms);

// Flow Control frame: continue, no block limit, no minimum separation, padded to 8.
void isotp_flow_control(uint8_t out[8]);
