// obd_client task: engine ECU DTC status over OBD-II (SPEC 8).
#pragma once

#include "esp_err.h"

// Start the obd_client task. Does nothing in listen-only mode (OBD requests
// need TWAI normal mode, SPEC 8.5). Call after can_link_start().
esp_err_t obd_client_start(void);
