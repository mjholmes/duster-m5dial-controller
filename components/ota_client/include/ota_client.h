// ota_client: the ota task, owner of the update check/install flow
// (docs/ota-update-spec.md 8). Drives wifi_link for connectivity; never
// touches esp_wifi directly.
#pragma once

#include <stdint.h>
#include "esp_err.h"

typedef enum {
    OTA_CLIENT_IDLE,
    OTA_CLIENT_CONNECTING_WIFI,
    OTA_CLIENT_CHECKING,           // fetching/parsing the manifest
    OTA_CLIENT_UP_TO_DATE,
    OTA_CLIENT_UPDATE_AVAILABLE,   // waiting for an install request
    OTA_CLIENT_DOWNLOADING,
    OTA_CLIENT_REBOOTING,
    OTA_CLIENT_FAILED,
} ota_client_state_t;

// Starts the ota task. Call once from app_main(), after wifi_link_start().
esp_err_t ota_client_start(void);

// Ask the ota task to connect Wi-Fi and check the manifest for the
// selected channel (docs/ota-update-spec.md 5, 8 steps 1-2). Non-blocking.
void ota_client_request_check(void);

// Ask the ota task to download and install the version found by the last
// check (8 steps 3-6). Only meaningful from OTA_CLIENT_UPDATE_AVAILABLE;
// ignored otherwise. Non-blocking.
void ota_client_request_install(void);

// Any task.
ota_client_state_t ota_client_get_state(void);

// Version string found by the last check, valid once state reaches
// OTA_CLIENT_UPDATE_AVAILABLE. Empty otherwise.
const char *ota_client_get_available_version(void);

// Download progress in percent (0-100), valid while OTA_CLIENT_DOWNLOADING.
int ota_client_get_progress_pct(void);

// Short, human-readable reason for the last failure (8/9). Empty if the
// last attempt did not fail.
const char *ota_client_get_fail_reason(void);
