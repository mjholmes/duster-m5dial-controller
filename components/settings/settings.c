#include "settings.h"

#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "settings";
static const char *NVS_NAMESPACE = "settings";
// Index 0 = day, 1 = night.
static const char *const KEY_BRIGHTNESS[2] = { "bright_day", "bright_night" };
// Single-level key from before day/night levels; read once as the day value.
static const char *KEY_BRIGHTNESS_OLD = "bright_pct";

static int s_brightness_pct[2] = {
    CONFIG_DISPLAY_BRIGHTNESS_DAY_DEFAULT_PCT,
    CONFIG_DISPLAY_BRIGHTNESS_NIGHT_DEFAULT_PCT,
};
static bool s_dirty;
static uint32_t s_changed_ms;

static bool brightness_valid(int pct)
{
    return pct >= CONFIG_DISPLAY_BRIGHTNESS_MIN_PCT && pct <= 100;
}

esp_err_t settings_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition unusable (%s), erasing", esp_err_to_name(err));
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        for (int i = 0; i < 2; i++) {
            uint8_t pct;
            if (nvs_get_u8(h, KEY_BRIGHTNESS[i], &pct) == ESP_OK && brightness_valid(pct)) {
                s_brightness_pct[i] = pct;
            } else if (i == 0 && nvs_get_u8(h, KEY_BRIGHTNESS_OLD, &pct) == ESP_OK && brightness_valid(pct)) {
                s_brightness_pct[0] = pct;
            }
        }
        nvs_close(h);
    }
    ESP_LOGI(TAG, "brightness day %d%%, night %d%%", s_brightness_pct[0], s_brightness_pct[1]);
    return ESP_OK;
}

int settings_brightness_pct(bool night)
{
    return s_brightness_pct[night];
}

void settings_set_brightness_pct(bool night, int pct)
{
    if (!brightness_valid(pct) || pct == s_brightness_pct[night]) {
        return;
    }
    s_brightness_pct[night] = pct;
    s_dirty = true;
    s_changed_ms = (uint32_t)(esp_timer_get_time() / 1000);
}

void settings_service(uint32_t now_ms)
{
    if (!s_dirty || (uint32_t)(now_ms - s_changed_ms) < CONFIG_SETTINGS_SAVE_DELAY_MS) {
        return;
    }
    s_dirty = false;

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err == ESP_OK) {
        for (int i = 0; i < 2 && err == ESP_OK; i++) {
            err = nvs_set_u8(h, KEY_BRIGHTNESS[i], (uint8_t)s_brightness_pct[i]);
        }
        if (err == ESP_OK) {
            err = nvs_commit(h);
        }
        nvs_close(h);
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "save failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "saved brightness day %d%%, night %d%%", s_brightness_pct[0], s_brightness_pct[1]);
    }
}
