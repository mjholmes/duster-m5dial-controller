// aux_link: the aux task, owner of the RA60 library (SPEC 7.2).
#pragma once

#include "esp_err.h"

// Start the aux task. Call after board_init() and control_start().
esp_err_t aux_link_start(void);
