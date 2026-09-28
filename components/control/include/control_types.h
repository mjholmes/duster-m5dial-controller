// Shared types for control, the UI model and requests (SPEC 5, 8.4, 4.2).
// Plain C, no FreeRTOS, so the logic can be tested off-target.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { MODE_MANUAL = 0, MODE_AUTO = 1 } op_mode_t;

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
    // DTC
    dtc_state_t dtc;
} ui_model_t;
