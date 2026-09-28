// CAN link: TWAI setup and the can_rx task (SPEC 5, 5.1, 8.5).
#pragma once

#include "esp_err.h"

// Configure TWAI on Port A and start can_rx. Call after board_init() and
// control_start().
esp_err_t can_link_start(void);
