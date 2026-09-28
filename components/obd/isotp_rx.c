#include "isotp_rx.h"

#include <string.h>

#define PCI_SF 0x0
#define PCI_FF 0x1
#define PCI_CF 0x2

void isotp_rx_reset(isotp_rx_t *rx)
{
    memset(rx, 0, sizeof(*rx));
}

static void store(isotp_rx_t *rx, const uint8_t *src, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        if (rx->len < ISOTP_RX_MAX) {
            rx->buf[rx->len++] = src[i];
        }
    }
    rx->received += n;
}

isotp_rx_result_t isotp_rx_feed(isotp_rx_t *rx, const uint8_t *data, uint8_t dlc, uint32_t now_ms)
{
    if (dlc < 1) {
        return ISOTP_RX_NONE;
    }
    uint8_t pci = data[0] >> 4;

    switch (pci) {
    case PCI_SF: {
        uint8_t n = data[0] & 0x0F;
        if (n == 0 || n > 7 || n > dlc - 1) {
            isotp_rx_reset(rx);
            return ISOTP_RX_ERROR;
        }
        isotp_rx_reset(rx);
        rx->total = n;
        store(rx, data + 1, n);
        return ISOTP_RX_DONE;
    }

    case PCI_FF: {
        if (dlc < 8) {
            isotp_rx_reset(rx);
            return ISOTP_RX_ERROR;
        }
        uint16_t total = (uint16_t)(((data[0] & 0x0F) << 8) | data[1]);
        if (total < 8) {
            isotp_rx_reset(rx);
            return ISOTP_RX_ERROR;
        }
        isotp_rx_reset(rx);
        rx->total = total;
        store(rx, data + 2, 6);
        rx->next_sn = 1;
        rx->active = true;
        rx->last_ms = now_ms;
        return ISOTP_RX_SEND_FC;
    }

    case PCI_CF: {
        if (!rx->active) {
            return ISOTP_RX_NONE;   // stray CF with no message in progress
        }
        uint8_t sn = data[0] & 0x0F;
        if (sn != rx->next_sn) {
            isotp_rx_reset(rx);     // gap or out of order: abort (SPEC 8.2)
            return ISOTP_RX_ERROR;
        }
        uint16_t remaining = rx->total - rx->received;
        uint16_t n = remaining < 7 ? remaining : 7;
        if (n > dlc - 1) {
            isotp_rx_reset(rx);
            return ISOTP_RX_ERROR;
        }
        store(rx, data + 1, n);
        rx->next_sn = (rx->next_sn + 1) & 0x0F;   // 1..F then 0
        rx->last_ms = now_ms;
        if (rx->received >= rx->total) {
            rx->active = false;
            return ISOTP_RX_DONE;
        }
        return ISOTP_RX_NONE;
    }

    default:
        return ISOTP_RX_NONE;   // flow control or unknown PCI: not for a receiver
    }
}

bool isotp_rx_check_timeout(isotp_rx_t *rx, uint32_t now_ms, uint32_t cf_timeout_ms)
{
    if (rx->active && (uint32_t)(now_ms - rx->last_ms) >= cf_timeout_ms) {
        isotp_rx_reset(rx);
        return true;
    }
    return false;
}

void isotp_flow_control(uint8_t out[8])
{
    const uint8_t fc[8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(out, fc, 8);
}
