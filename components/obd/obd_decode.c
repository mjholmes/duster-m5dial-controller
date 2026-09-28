#include "obd_decode.h"

static const char HEX[] = "0123456789ABCDEF";

bool obd_decode_pid01(const uint8_t *p, uint16_t len, bool *mil_on, uint8_t *count)
{
    if (len < 3 || p[0] != 0x41 || p[1] != 0x01) {
        return false;
    }
    *mil_on = (p[2] & 0x80) != 0;
    *count = p[2] & 0x7F;
    return true;
}

void obd_dtc_to_string(uint8_t b0, uint8_t b1, char out[6])
{
    out[0] = "PCBU"[(b0 >> 6) & 0x03];
    out[1] = HEX[(b0 >> 4) & 0x03];
    out[2] = HEX[b0 & 0x0F];
    out[3] = HEX[(b1 >> 4) & 0x0F];
    out[4] = HEX[b1 & 0x0F];
    out[5] = '\0';
}

int obd_decode_mode03(const uint8_t *p, uint16_t len, char codes[][6], int max_codes)
{
    if (len < 1 || p[0] != 0x43) {
        return -1;
    }
    // Layout by length: 43 n + 2n bytes (even total) or 43 + 2k bytes (odd).
    uint16_t first = (len % 2 == 0) ? 2 : 1;
    int written = 0;
    for (uint16_t i = first; i + 1 < len && written < max_codes; i += 2) {
        if (p[i] == 0 && p[i + 1] == 0) {
            continue;   // padding
        }
        obd_dtc_to_string(p[i], p[i + 1], codes[written++]);
    }
    return written;
}
