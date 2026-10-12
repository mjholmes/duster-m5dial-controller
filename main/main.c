// Duster M5Dial controller: boot, init order, task start.

#include "board_m5dial.h"
#include "ui_bridge.h"
#include "control.h"
#include "settings.h"
#include "can_link.h"
#include "aux_link.h"
#include "obd_client.h"
#include "health.h"
#include "wifi_link.h"
#include "ota_client.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"

static const char *TAG = "main";

static void log_heap(const char *when)
{
    // Phase 2 headroom early warning (SPEC 4.2 item 7).
    ESP_LOGI(TAG, "heap %s: free internal %u, largest block %u", when,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

void app_main(void)
{
    board_hold_on();
    health_log_reset_reason();
    log_heap("boot");

    // Must run before TWAI (Port A) and the aux UART (Port B) are set up:
    // board_init() resets those pads and checks M5Unified leaves them alone.
    ESP_ERROR_CHECK(board_init());
    ESP_ERROR_CHECK(settings_init());
    ESP_ERROR_CHECK(control_start());
    ESP_ERROR_CHECK(can_link_start());

    // Self-test (docs/ota-update-spec.md 8 step 7): TWAI and the display
    // are confirmed working by the two ESP_ERROR_CHECK calls above having
    // not aborted. No separate 60 s timer is needed -- this point is
    // reached synchronously, within a second of boot, and the existing
    // 3 s task watchdog (SPEC 5) already forces a reset (and so a
    // rollback, while still pending-verify) on anything that hangs before
    // it.
    esp_err_t mark_err = esp_ota_mark_app_valid_cancel_rollback();
    if (mark_err != ESP_OK) {
        ESP_LOGW(TAG, "mark app valid failed: %s", esp_err_to_name(mark_err));
    }

    ESP_ERROR_CHECK(aux_link_start());
    ESP_ERROR_CHECK(obd_client_start());
    ESP_ERROR_CHECK(wifi_link_start());
    ESP_ERROR_CHECK(ota_client_start());
    ESP_ERROR_CHECK(ui_bridge_start());

    vTaskDelay(pdMS_TO_TICKS(2000));
    log_heap("idle");
    // Spike check 3: Port A (G13/G15) and Port B (G1/G2) must be unconfigured.
    gpio_dump_io_configuration(stdout, (1ULL << 1) | (1ULL << 2) | (1ULL << 13) | (1ULL << 15));
}
