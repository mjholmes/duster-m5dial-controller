// wifi_link: the wifi task, sole owner of the esp_wifi/esp_netif driver
// and the esp_console REPL (docs/ota-update-spec.md 3). Wi-Fi is started
// only on request and stopped once connected/failed/disconnected -- it
// never runs during normal operation.
#pragma once

#include <stddef.h>
#include "esp_err.h"

typedef enum {
    WIFI_LINK_IDLE,            // Wi-Fi stopped, no connection attempted (or a clean disconnect)
    WIFI_LINK_NOT_CONFIGURED,  // no credentials in NVS or Kconfig default
    WIFI_LINK_CONNECTING,
    WIFI_LINK_CONNECTED,
    WIFI_LINK_FAILED,          // auth failure or connect timeout
} wifi_link_state_t;

// Starts the wifi task (owns esp_wifi/esp_netif) and the USB-Serial-JTAG
// esp_console REPL (`wifi set/show/clear`). Call once from app_main().
esp_err_t wifi_link_start(void);

// Ask the wifi task to connect using the stored credentials (NVS, falling
// back to the Kconfig build-time default). Non-blocking; poll
// wifi_link_get_state() for the result.
void wifi_link_request_connect(void);

// Ask the wifi task to disconnect and stop the Wi-Fi driver. Non-blocking.
void wifi_link_request_disconnect(void);

// Any task.
wifi_link_state_t wifi_link_get_state(void);
