// control task: single writer of mode and commanded light state (SPEC 4.2, 6).
#pragma once

#include "control_types.h"
#include "esp_err.h"

esp_err_t control_start(void);

// Queue a user request (any task). Returns false if ctrl_req_q is full (dropped, logged).
bool control_request(const ctrl_req_t *req);

// Vehicle data from can_rx (SPEC 5.1). Scalars go straight into the model;
// light inputs are timestamped on every frame (link timeout) and queued to
// control only when they change.
void control_can_speed(uint8_t mph, uint32_t rx_ms);
void control_can_temp(int8_t temp_c, uint32_t rx_ms);
void control_can_ignition(uint8_t raw);
void control_can_lights(vehicle_lights_t lights, bool changed, uint32_t rx_ms);
void control_can_bus_off(bool bus_off);

// RA60 link (SPEC 7.2), used only by the aux task. Newest commanded target
// since the last call, if any; and ACKed / failed results back to control.
bool control_aux_take_target(aux_lights_t *out);
void control_aux_event(bool acked, aux_lights_t lights, uint32_t at_ms);

// Engine DTC status from the obd_client task (SPEC 8.4).
void control_obd_dtc(const dtc_state_t *dtc);

// Consistent copy of everything a display shows. Any task.
void model_snapshot(ui_model_t *out);

// Next request sequence number for a local source.
uint16_t control_next_seq(void);
