// control task: single writer of mode and commanded light state (SPEC 4.2, 6).
#pragma once

#include "control_types.h"
#include "esp_err.h"

esp_err_t control_start(void);

// Queue a user request (any task). Returns false if ctrl_req_q is full (dropped, logged).
bool control_request(const ctrl_req_t *req);

// Consistent copy of everything a display shows. Any task.
void model_snapshot(ui_model_t *out);

// Next request sequence number for a local source.
uint16_t control_next_seq(void);
