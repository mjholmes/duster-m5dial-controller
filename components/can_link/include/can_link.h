// CAN link: TWAI setup and the can_rx task (SPEC 5, 5.1, 8.5).
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Configure TWAI on Port A and start can_rx. Call after board_init() and
// control_start().
esp_err_t can_link_start(void);

// Queue a classic 11-bit frame, DLC 8. Any task. ESP_ERR_NOT_SUPPORTED in
// listen-only mode, ESP_ERR_INVALID_STATE while bus-off, ESP_ERR_TIMEOUT if
// no transmit slot is free.
esp_err_t can_link_send(uint32_t id, const uint8_t data[8]);

// Engine ECU OBD responses (0x7E8), routed from can_rx (obd_rx_q). For the
// obd_client task only.
bool can_link_obd_receive(uint8_t data[8], uint32_t wait_ms);
