// Shared types for control, the UI model and requests (SPEC 5, 8.4, 4.2).
// Plain C, no FreeRTOS, so the logic can be tested off-target.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { MODE_MANUAL = 0, MODE_AUTO = 1 } op_mode_t;

// Canonical wheel-position index, shared by the TPMS/wheel-speed decode
// output and the model. The frames themselves are in a different order
// (0x673 TPMS: RR,RL,FR,FL; 0x29A/0x29C wheel speed: FR,FL / RL,RR) -- the
// remapping to this order happens only in can_signals.c (docs/ui-tyres-4wd-spec.md 4).
typedef enum { TYRE_FL = 0, TYRE_FR, TYRE_RL, TYRE_RR, TYRE_COUNT } tyre_pos_t;

typedef enum { REQ_SRC_LOCAL_UI = 0, REQ_SRC_REMOTE_DISPLAY, REQ_SRC_BUTTON_REMOTE } req_src_t;
typedef enum { AUX_LIGHT_LEDBAR = 0, AUX_LIGHT_FOG = 1 } aux_light_id_t;
typedef enum { REQ_SET_MODE, REQ_SET_LIGHT, REQ_TOGGLE_LIGHT, REQ_MANUAL_ALL_OFF } req_type_t;

typedef struct {
    req_type_t     type;
    req_src_t      src;
    uint16_t       seq;
    op_mode_t      mode;      // REQ_SET_MODE
    aux_light_id_t light;     // REQ_SET_LIGHT / REQ_TOGGLE_LIGHT
    bool           on;        // REQ_SET_LIGHT
} ctrl_req_t;

typedef struct {
    bool main_beam;
    bool front_fog;
    bool dipped_beam;   // 0x5DE byte 0 bit 1; display night dim only
} vehicle_lights_t;

typedef struct {
    bool ledbar;    // LED bar, library output_index 1, follows main beam in AUTO
    bool fog;       // aux fog, library output_index 2, follows front fogs in AUTO
} aux_lights_t;

typedef enum {
    DTC_NOT_READ = 0, DTC_READING, DTC_NONE, DTC_PRESENT_NO_MIL, DTC_PRESENT_MIL, DTC_READ_FAILED
} dtc_status_t;

typedef struct {
    dtc_status_t status;
    bool     mil_on;
    uint8_t  count;          // from PID 01
    uint8_t  codes_read;     // from Mode 03, may be < count if truncated
    char     codes[8][6];    // "P0301\0", cap at 8
} dtc_state_t;

typedef enum { REMOTE_LINK_NOT_CONFIGURED = 0, REMOTE_LINK_UP = 1, REMOTE_LINK_LOST = 2 } remote_link_t;

// Tyre pressure / wheel speed / 4WD model (docs/ui-tyres-4wd-spec.md 4).
// Raw units; conversion and formatting happen in ui_bridge.
typedef struct {
    uint8_t  raw[TYRE_COUNT];   // tyre_pos_t index; 0xFF = no value
    uint8_t  state[TYRE_COUNT]; // tyre_pos_t index; 3-bit wheel state (0 OK, 1 no info, 5 flat, 6 under-inflated) --
                                 // not in the spec's tyre_pressures_t, added because section 9's
                                 // is_tyre_warning explicitly needs it
    bool     failure;           // TpmsFailureWarning -- same addition, same reason (OQ-29)
    bool     stale;             // 0x673 not seen within CONFIG_TPMS_STALE_MS
} tyre_pressures_t;

typedef struct {
    uint16_t raw[TYRE_COUNT];      // tyre_pos_t index, x 0.0417 rpm
    uint16_t speed_centi_kmh;      // 0x29A bytes 4-5
    bool     stale;                // either 0x29A or 0x29C stale
} wheel_speeds_t;

typedef struct {
    uint16_t torque_nm;
    uint8_t  coupler;   // 0 open, 1 sliding, 2 closed, 3 unavailable
    uint8_t  mode;      // 0 auto, 1 fixed 4WD, 2 fixed 2WD, 3 unavailable
    uint8_t  selector;  // 0 AUTO, 1 LOCK, 2 2WD, 3 unavailable
    bool     failure;
    bool     stale;
} etc_state_t;

// Everything a display shows (SPEC 4.2 items 2 and 6). Filled by
// model_snapshot(); Phase 2 telemetry is built from the same struct.
typedef struct {
    // vehicle
    uint8_t  speed_mph;           // 0..99, clamped
    int8_t   ext_temp_c;
    bool     speed_stale;
    bool     temp_stale;
    bool     frost_risk;          // hysteresis, false when temp stale
    bool     below_freezing;      // hysteresis, false when temp stale
    uint8_t  ignition_raw;        // 0x350 byte 0, 0 = not yet seen
    vehicle_lights_t veh;
    // control
    op_mode_t mode;
    uint16_t last_applied_req_seq;
    aux_lights_t manual_state;
    aux_lights_t aux_commanded;
    aux_lights_t aux_actual;      // last ACKed state; RA60 has no status readback
    bool     aux_fault;
    bool     aux_mismatch;
    // link health
    bool     can_timeout;
    bool     can_bus_off;
    bool     aux_timeout;
    remote_link_t remote_link;    // always NOT_CONFIGURED in Phase 1
    // tyres / 4WD (docs/ui-tyres-4wd-spec.md)
    tyre_pressures_t tyre_pressures;
    wheel_speeds_t    wheel_speeds;
    etc_state_t       etc;
    // DTC
    dtc_state_t dtc;
} ui_model_t;
