#include "can_signals.h"

#include <stddef.h>

// One entry per consumed DBC message (docs/can/dacia-duster-2015-testing.dbc).
static const struct {
    uint32_t    id;
    uint8_t     dlc;     // DBC DLC; shorter frames are discarded and counted
    const char *name;
} s_table[CAN_SIG_COUNT] = {
    [CAN_SIG_SPEED]    = { 0x354, 2, "SPEED" },           // BO_ 852: bytes 0-1 BE u16 x 0.01 km/h
    [CAN_SIG_TEMP]     = { 0x3B7, 6, "AMBIENT_TEMP" },    // BO_ 951: byte 0 - 40 = degC
    [CAN_SIG_LIGHTS]   = { 0x5DE, 8, "LIGHTS" },          // BO_ 1502: byte 0 bits 0 MainBeam, 1 DippedBeam, 3 FrontFog
    [CAN_SIG_IGNITION] = { 0x350, 8, "IGNITION_STATUS" }, // BO_ 848: byte 0 enum 0xC3..0xC7
    [CAN_SIG_TPMS]              = { 0x673, 7, "TPMS_PRESSURES" },    // BO_ 1651
    [CAN_SIG_WHEEL_SPEED_FRONT] = { 0x29A, 8, "WHEEL_SPEED_FRONT" }, // BO_ 666
    [CAN_SIG_WHEEL_SPEED_REAR]  = { 0x29C, 8, "WHEEL_SPEED_REAR" },  // BO_ 668
    [CAN_SIG_ETC]               = { 0x1AA, 4, "ETC_STATUS" },        // BO_ 426
};

#define OBD_ENGINE_RESPONSE_ID 0x7E8

#define IGNITION_MIN 0xC3
#define IGNITION_MAX 0xC7
#define SPEED_MPH_MAX 99

// km/h = raw / 100; mph = km/h / 1.609344 = raw / 160.9344, rounded.
static uint8_t centi_kmh_to_mph(uint16_t raw)
{
    uint32_t mph = ((uint32_t)raw * 10000u + 804672u) / 1609344u;
    return mph > SPEED_MPH_MAX ? SPEED_MPH_MAX : (uint8_t)mph;
}

can_dec_result_t can_decode(uint32_t id, bool extended, bool rtr,
                            const uint8_t *data, uint8_t len, can_decoded_t *out)
{
    // The vehicle bus is CAN 2.0A (11-bit IDs).
    if (extended) {
        return CAN_DEC_NOT_SUBSCRIBED;
    }
    int sig = -1;
    for (int i = 0; i < CAN_SIG_COUNT; i++) {
        if (s_table[i].id == id) {
            sig = i;
            break;
        }
    }
    if (sig < 0) {
        return CAN_DEC_NOT_SUBSCRIBED;
    }
    out->sig = (can_sig_t)sig;
    if (rtr || len < s_table[sig].dlc) {
        return CAN_DEC_MALFORMED;
    }

    switch (out->sig) {
    case CAN_SIG_SPEED:
        out->speed.raw_centi_kmh = (uint16_t)((data[0] << 8) | data[1]);
        out->speed.mph = centi_kmh_to_mph(out->speed.raw_centi_kmh);
        return CAN_DEC_OK;

    case CAN_SIG_TEMP:
        out->ext_temp_c = (int8_t)((int)data[0] - 40);
        return CAN_DEC_OK;

    case CAN_SIG_LIGHTS:
        out->lights = (vehicle_lights_t){
            .main_beam   = (data[0] & 0x01) != 0,
            .dipped_beam = (data[0] & 0x02) != 0,
            .front_fog   = (data[0] & 0x08) != 0,
        };
        return CAN_DEC_OK;

    case CAN_SIG_IGNITION:
        out->ignition_raw = data[0];
        if (data[0] < IGNITION_MIN || data[0] > IGNITION_MAX) {
            return CAN_DEC_UNKNOWN_VALUE;
        }
        return CAN_DEC_OK;

    case CAN_SIG_TPMS:
        out->tpms.failure           = (data[0] >> 6) & 1;
        out->tpms.state[TYRE_RR]    = (data[0] >> 3) & 7;
        out->tpms.state[TYRE_RL]    = data[0] & 7;
        out->tpms.state[TYRE_FR]    = data[1] >> 5;
        out->tpms.state[TYRE_FL]    = (data[1] >> 2) & 7;
        out->tpms.pressure_raw[TYRE_RR] = data[2];
        out->tpms.pressure_raw[TYRE_RL] = data[3];
        out->tpms.pressure_raw[TYRE_FR] = data[4];
        out->tpms.pressure_raw[TYRE_FL] = data[5];
        return CAN_DEC_OK;

    case CAN_SIG_WHEEL_SPEED_FRONT:
        out->wheel_speed_front.fr_raw = (uint16_t)((data[0] << 8) | data[1]);
        out->wheel_speed_front.fl_raw = (uint16_t)((data[2] << 8) | data[3]);
        out->wheel_speed_front.speed_raw_centi_kmh = (uint16_t)((data[4] << 8) | data[5]);
        return CAN_DEC_OK;

    case CAN_SIG_WHEEL_SPEED_REAR:
        out->wheel_speed_rear.rl_raw = (uint16_t)((data[0] << 8) | data[1]);
        out->wheel_speed_rear.rr_raw = (uint16_t)((data[2] << 8) | data[3]);
        return CAN_DEC_OK;

    case CAN_SIG_ETC:
        out->etc.torque_nm = (uint16_t)((((data[2] & 0x7F) << 1) | (data[3] >> 7)) * 10);
        out->etc.coupler  = data[3] & 0x03;
        out->etc.mode     = (data[3] >> 4) & 0x03;
        out->etc.selector = (data[3] >> 2) & 0x03;
        out->etc.failure  = (data[3] >> 6) & 0x01;
        return CAN_DEC_OK;

    default:
        return CAN_DEC_NOT_SUBSCRIBED;
    }
}

const char *can_sig_name(can_sig_t sig)
{
    return sig < CAN_SIG_COUNT ? s_table[sig].name : "?";
}

uint32_t can_sig_id(can_sig_t sig)
{
    return sig < CAN_SIG_COUNT ? s_table[sig].id : 0;
}

bool can_is_obd_response(uint32_t id)
{
    return id == OBD_ENGINE_RESPONSE_ID;
}
