#include "ota_client.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_app_desc.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "settings.h"
#include "wifi_link.h"
#include "sdkconfig.h"

static const char *TAG = "ota_client";

#define OTA_TASK_CORE  1
#define OTA_TASK_PRIO  2
#define OTA_TASK_STACK 6144
#define OTA_TASK_PERIOD_MS 200
#define MANIFEST_MAX_LEN 4096

typedef enum { OTA_REQ_CHECK, OTA_REQ_INSTALL } ota_req_t;

static QueueHandle_t s_req_q;
static volatile ota_client_state_t s_state = OTA_CLIENT_IDLE;
static char s_fail_reason[48];
static char s_available_version[32];
static char s_pending_url[256];
static size_t s_pending_size;
static volatile int s_progress_pct;
static volatile bool s_cancel_requested;

static void set_failed(const char *reason)
{
    snprintf(s_fail_reason, sizeof(s_fail_reason), "%s", reason);
    s_state = OTA_CLIENT_FAILED;
    wifi_link_request_disconnect();
}

// GitHub issue #4: a cooperative check for the two blocking calls (do_check
// and do_install's download loop) -- ota_task owns s_state/s_cancel_requested,
// so this always runs on ota_task, never racing the request from ui_task.
static bool check_cancelled(void)
{
    if (!s_cancel_requested) {
        return false;
    }
    s_cancel_requested = false;
    s_state = OTA_CLIENT_IDLE;
    wifi_link_request_disconnect();
    return true;
}

// --- Manifest (docs/ota-update-spec.md 5, 6.2) ---

static esp_err_t fetch_manifest(char *out, size_t out_len)
{
    esp_http_client_config_t config = {
        .url = CONFIG_OTA_MANIFEST_URL,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = CONFIG_OTA_HTTP_BUFFER_SIZE,
        .buffer_size_tx = CONFIG_OTA_HTTP_BUFFER_SIZE_TX,
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "manifest open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }
    int content_length = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "manifest headers: status=%d content_length=%d", status, content_length);
    if (content_length <= 0 || (size_t)content_length >= out_len) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    int read_len = esp_http_client_read(client, out, content_length);
    out[read_len > 0 ? read_len : 0] = '\0';
    ESP_LOGI(TAG, "manifest read_len=%d body=%s", read_len, out);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    if (read_len != content_length || status != 200) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

// Parses the manifest and selects the entry for the current channel.
// *out_version/out_url are borrowed from `root` -- copy before it's freed.
static esp_err_t parse_manifest(cJSON *root, const char **out_version, const char **out_url, size_t *out_size)
{
    cJSON *schema = cJSON_GetObjectItem(root, "schema");
    if (!cJSON_IsNumber(schema) || schema->valueint != 1) {
        return ESP_FAIL;
    }
    const char *channel = settings_ota_beta_channel() ? "beta" : "stable";
    cJSON *entry = cJSON_GetObjectItem(root, channel);
    cJSON *version = cJSON_GetObjectItem(entry, "version");
    cJSON *url = cJSON_GetObjectItem(entry, "url");
    cJSON *size = cJSON_GetObjectItem(entry, "size");
    if (!cJSON_IsString(version) || !cJSON_IsString(url) || !cJSON_IsNumber(size)) {
        return ESP_FAIL;
    }
    *out_version = version->valuestring;
    *out_url = url->valuestring;
    *out_size = (size_t)size->valueint;
    return ESP_OK;
}

static void do_check(void)
{
    if (CONFIG_OTA_MANIFEST_URL[0] == '\0') {
        set_failed("Manifest not configured");
        return;
    }

    char *body = malloc(MANIFEST_MAX_LEN);
    if (!body) {
        set_failed("Out of memory");
        return;
    }
    esp_err_t err = fetch_manifest(body, MANIFEST_MAX_LEN);
    if (check_cancelled()) {
        free(body);
        return;
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "manifest fetch failed: %s", esp_err_to_name(err));
        free(body);
        set_failed("Manifest fetch failed");
        return;
    }

    cJSON *root = cJSON_Parse(body);
    free(body);
    if (!root) {
        set_failed("Manifest invalid");
        return;
    }
    const char *version, *url;
    size_t size;
    if (parse_manifest(root, &version, &url, &size) != ESP_OK) {
        cJSON_Delete(root);
        set_failed("Manifest invalid");
        return;
    }

    const char *running_version = esp_app_get_description()->version;
    if (strcmp(version, running_version) == 0) {
        cJSON_Delete(root);
        s_state = OTA_CLIENT_UP_TO_DATE;
        wifi_link_request_disconnect();
        return;
    }

    snprintf(s_available_version, sizeof(s_available_version), "%s", version);
    snprintf(s_pending_url, sizeof(s_pending_url), "%s", url);
    s_pending_size = size;
    cJSON_Delete(root);
    s_state = OTA_CLIENT_UPDATE_AVAILABLE;
    // Wi-Fi stays up -- the install the user is about to confirm needs it.
}

// --- Install (docs/ota-update-spec.md 8 steps 4-6) ---

static void do_install(void)
{
    s_state = OTA_CLIENT_DOWNLOADING;
    s_progress_pct = 0;

    esp_http_client_config_t http_config = {
        .url = s_pending_url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = CONFIG_OTA_HTTP_BUFFER_SIZE,
        .buffer_size_tx = CONFIG_OTA_HTTP_BUFFER_SIZE_TX,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = { .http_config = &http_config };

    esp_https_ota_handle_t handle;
    esp_err_t err = esp_https_ota_begin(&ota_config, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "ota begin failed: %s", esp_err_to_name(err));
        set_failed("Download failed to start");
        return;
    }

    // Flash erase/write briefly stalls both CPU cores (ESP32 cache-safety
    // mechanism); a few hundred of these back to back, on top of the ui
    // task's own CPU1 load, can starve the idle tasks past the normal 3 s
    // task watchdog (SPEC 5 tuned that for fast vehicle-safety fault
    // detection, not for this). Widen it only for the download itself.
    const esp_task_wdt_config_t wide_wdt = { .timeout_ms = 20000, .idle_core_mask = 0x3, .trigger_panic = true };
    const esp_task_wdt_config_t normal_wdt = { .timeout_ms = CONFIG_ESP_TASK_WDT_TIMEOUT_S * 1000, .idle_core_mask = 0x3, .trigger_panic = true };
    esp_task_wdt_reconfigure(&wide_wdt);

    for (;;) {
        if (s_cancel_requested) {
            esp_https_ota_abort(handle);
            esp_task_wdt_reconfigure(&normal_wdt);
            check_cancelled();  // always true here; resets state and clears the flag
            return;
        }
        err = esp_https_ota_perform(handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        if (s_pending_size > 0) {
            size_t read = (size_t)esp_https_ota_get_image_len_read(handle);
            s_progress_pct = (int)((read * 100) / s_pending_size);
        }
    }

    if (err != ESP_OK || !esp_https_ota_is_complete_data_received(handle)) {
        ESP_LOGW(TAG, "ota download interrupted: %s", esp_err_to_name(err));
        esp_https_ota_abort(handle);
        esp_task_wdt_reconfigure(&normal_wdt);
        set_failed("Download interrupted");
        return;
    }

    esp_err_t finish_err = esp_https_ota_finish(handle);
    esp_task_wdt_reconfigure(&normal_wdt);
    if (finish_err != ESP_OK) {
        ESP_LOGW(TAG, "ota finish failed: %s", esp_err_to_name(finish_err));
        set_failed(finish_err == ESP_ERR_OTA_VALIDATE_FAILED ? "Signature check failed" : "Install failed");
        return;
    }

    wifi_link_request_disconnect();
    s_state = OTA_CLIENT_REBOOTING;
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}

// --- Task ---

// Deliberately not registered with the task watchdog (unlike the other
// tasks, SPEC 5): a TLS handshake or a slow link can legitimately block
// one loop iteration past CONFIG_ESP_TASK_WDT_TIMEOUT_S (3 s), and a hang
// here isn't a vehicle-safety condition the way control/aux/can are.
static void ota_task(void *arg)
{
    (void)arg;
    for (;;) {
        // CHECKING/DOWNLOADING only ever exist for the duration of the
        // synchronous do_check()/do_install() call within one iteration
        // (see below) -- by the time we're back at the top of the loop,
        // state is always something else, so a plain reset here is safe.
        if (s_cancel_requested) {
            s_cancel_requested = false;
            wifi_link_request_disconnect();
            s_state = OTA_CLIENT_IDLE;
        }

        ota_req_t req;
        if (xQueueReceive(s_req_q, &req, 0) == pdTRUE) {
            if (req == OTA_REQ_CHECK) {
                s_fail_reason[0] = '\0';
                s_state = OTA_CLIENT_CONNECTING_WIFI;
                wifi_link_request_connect();
            } else if (req == OTA_REQ_INSTALL && s_state == OTA_CLIENT_UPDATE_AVAILABLE) {
                do_install();
            }
        }

        if (s_state == OTA_CLIENT_CONNECTING_WIFI) {
            switch (wifi_link_get_state()) {
            case WIFI_LINK_CONNECTED:
                s_state = OTA_CLIENT_CHECKING;
                do_check();
                break;
            case WIFI_LINK_NOT_CONFIGURED:
                set_failed("Wi-Fi not configured");
                break;
            case WIFI_LINK_FAILED:
                set_failed("Wi-Fi not available");
                break;
            default:
                break;  // still connecting
            }
        }

        vTaskDelay(pdMS_TO_TICKS(OTA_TASK_PERIOD_MS));
    }
}

esp_err_t ota_client_start(void)
{
    s_req_q = xQueueCreate(2, sizeof(ota_req_t));
    if (!s_req_q) {
        return ESP_ERR_NO_MEM;
    }

    // docs/ota-update-spec.md 7: "a notice that the update was rolled
    // back." Shown as the initial status until the user next interacts
    // with the screen (checks or installs), which naturally moves the
    // state on.
    if (esp_ota_get_last_invalid_partition() != NULL) {
        snprintf(s_fail_reason, sizeof(s_fail_reason), "Previous update rolled back");
        s_state = OTA_CLIENT_FAILED;
    }

    BaseType_t ok = xTaskCreatePinnedToCore(ota_task, "ota", OTA_TASK_STACK, NULL, OTA_TASK_PRIO, NULL, OTA_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

void ota_client_request_check(void)
{
    ota_req_t req = OTA_REQ_CHECK;
    xQueueSend(s_req_q, &req, 0);
}

void ota_client_request_install(void)
{
    ota_req_t req = OTA_REQ_INSTALL;
    xQueueSend(s_req_q, &req, 0);
}

void ota_client_request_cancel(void)
{
    s_cancel_requested = true;
}

void ota_client_notify_channel_changed(void)
{
    // GitHub issue #3: an UPDATE_AVAILABLE result names a specific
    // version/url for the *previous* channel; switching channels makes it
    // stale, so drop back to IDLE rather than letting Install use it.
    if (s_state == OTA_CLIENT_UPDATE_AVAILABLE) {
        s_available_version[0] = '\0';
        s_pending_url[0] = '\0';
        s_pending_size = 0;
        s_state = OTA_CLIENT_IDLE;
    }
}

ota_client_state_t ota_client_get_state(void) { return s_state; }
const char *ota_client_get_available_version(void) { return s_available_version; }
int ota_client_get_progress_pct(void) { return s_progress_pct; }
const char *ota_client_get_fail_reason(void) { return s_fail_reason; }
