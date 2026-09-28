// OBD-II response decoding (SPEC 8.1, 8.3). Pure C, host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "control_types.h"

// Mode 01 PID 01 positive response payload: 41 01 A B C D.
// A bit 7 = MIL, bits 0-6 = stored DTC count.
bool obd_decode_pid01(const uint8_t *p, uint16_t len, bool *mil_on, uint8_t *count);

// Mode 03 positive response payload: 43 [n] (b0 b1)*. Tolerates the count
// byte being present or not by the payload length (with it the length is
// even, without it odd). Writes up to max_codes "P0301" strings, skipping
// 0x0000 padding; returns the number written, or -1 if not a valid response.
int obd_decode_mode03(const uint8_t *p, uint16_t len, char codes[][6], int max_codes);

// One 2-byte DTC to "P0301" (6 bytes incl. terminator).
void obd_dtc_to_string(uint8_t b0, uint8_t b1, char out[6]);
