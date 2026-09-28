// Decode table for the broadcast CAN signals the firmware consumes (SPEC 5.1).
// Source: docs/can-signals.md, derived from the DBC. CAN IDs appear nowhere
// else in the codebase. Pure C, host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "control_types.h"

typedef enum {
    CAN_SIG_SPEED = 0,    // 0x354 SPEED
    CAN_SIG_TEMP,         // 0x3BF AMBIENT_TEMP
    CAN_SIG_LIGHTS,       // 0x5DE LIGHTS
    CAN_SIG_IGNITION,     // 0x350 IGNITION_STATUS
    CAN_SIG_COUNT
} can_sig_t;

typedef enum {
    CAN_DEC_OK = 0,
    CAN_DEC_NOT_SUBSCRIBED,   // ID not in the table
    CAN_DEC_MALFORMED,        // shorter than the DBC DLC, or RTR/extended
    CAN_DEC_UNKNOWN_VALUE,    // enum value outside the DBC's known set
} can_dec_result_t;

typedef struct {
    can_sig_t sig;
    union {
        struct {
            uint16_t raw_centi_kmh;   // 0.01 km/h per bit
            uint8_t  mph;             // rounded, clamped 0..99
        } speed;
        int8_t ext_temp_c;
        vehicle_lights_t lights;
        uint8_t ignition_raw;         // 0xC3..0xC7
    };
} can_decoded_t;

// Decode one classic frame. On CAN_DEC_MALFORMED and CAN_DEC_UNKNOWN_VALUE,
// out->sig is set so the caller can count per signal.
can_dec_result_t can_decode(uint32_t id, bool extended, bool rtr,
                            const uint8_t *data, uint8_t len, can_decoded_t *out);

// DBC message name and ID for a signal, for logging.
const char *can_sig_name(can_sig_t sig);
uint32_t can_sig_id(can_sig_t sig);

// Engine ECU OBD response ID (step 6 routes these to obd_rx_q).
bool can_is_obd_response(uint32_t id);
