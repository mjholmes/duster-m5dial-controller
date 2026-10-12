#include "wifi_link.h"

#include <string.h>

#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "health.h"
#include "sdkconfig.h"

static const char *TAG = "wifi_link";
static const char *NVS_NAMESPACE = "wifi";
static const char *KEY_SSID = "ssid";
static const char *KEY_PSK = "psk";

#define WIFI_TASK_CORE  1
#define WIFI_TASK_PRIO  2
#define WIFI_TASK_STACK 4096
#define WIFI_TASK_PERIOD_MS 200

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef enum { WIFI_REQ_CONNECT, WIFI_REQ_DISCONNECT } wifi_req_t;

static QueueHandle_t s_req_q;
static EventGroupHandle_t s_evt_group;
static volatile wifi_link_state_t s_state = WIFI_LINK_IDLE;
static bool s_driver_inited;
static uint32_t s_connect_start_ms;

// --- Credentials (NVS, docs/ota-update-spec.md 3.2) ---

static esp_err_t load_credentials(char *ssid, size_t ssid_len, char *psk, size_t psk_len)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
    if (err == ESP_OK) {
        size_t n = ssid_len;
        esp_err_t ssid_err = nvs_get_str(h, KEY_SSID, ssid, &n);
        n = psk_len;
        esp_err_t psk_err = nvs_get_str(h, KEY_PSK, psk, &n);
        nvs_close(h);
        if (ssid_err == ESP_OK && psk_err == ESP_OK && ssid[0] != '\0') {
            return ESP_OK;
        }
    }
    // Fall back to the Kconfig build-time default (local builds only).
    if (CONFIG_DUSTER_WIFI_DEFAULT_SSID[0] != '\0') {
        snprintf(ssid, ssid_len, "%s", CONFIG_DUSTER_WIFI_DEFAULT_SSID);
        snprintf(psk, psk_len, "%s", CONFIG_DUSTER_WIFI_DEFAULT_PSK);
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}

static esp_err_t save_credentials(const char *ssid, const char *psk)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_str(h, KEY_SSID, ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(h, KEY_PSK, psk);
    }
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

static esp_err_t clear_credentials(void)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    nvs_erase_key(h, KEY_SSID);
    nvs_erase_key(h, KEY_PSK);
    err = nvs_commit(h);
    nvs_close(h);
    return err;
}

// --- esp_wifi / esp_netif ---

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "disconnected");
        xEventGroupSetBits(s_evt_group, WIFI_FAIL_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *evt = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&evt->ip_info.ip));
        xEventGroupSetBits(s_evt_group, WIFI_CONNECTED_BIT);
    }
}

static void ensure_driver_init(void)
{
    if (s_driver_inited) {
        return;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    s_driver_inited = true;
}

static void begin_connect(void)
{
    char ssid[33] = { 0 };
    char psk[65] = { 0 };
    if (load_credentials(ssid, sizeof(ssid), psk, sizeof(psk)) != ESP_OK) {
        ESP_LOGW(TAG, "no credentials stored");
        s_state = WIFI_LINK_NOT_CONFIGURED;
        return;
    }

    ensure_driver_init();
    xEventGroupClearBits(s_evt_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    // ssid/password are fixed-size fields that need not be null-terminated
    // at their full length (32-byte SSIDs in particular) -- memcpy a
    // clamped length rather than snprintf, which would risk silently
    // truncating the last byte of a full-length value.
    wifi_config_t wifi_config = { 0 };
    memcpy(wifi_config.sta.ssid, ssid, strnlen(ssid, sizeof(wifi_config.sta.ssid)));
    memcpy(wifi_config.sta.password, psk, strnlen(psk, sizeof(wifi_config.sta.password)));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_connect failed: %s", esp_err_to_name(err));
        esp_wifi_stop();
        s_state = WIFI_LINK_FAILED;
        return;
    }

    s_connect_start_ms = (uint32_t)(esp_timer_get_time() / 1000);
    s_state = WIFI_LINK_CONNECTING;
}

static void stop_wifi(void)
{
    if (s_driver_inited) {
        esp_wifi_disconnect();
        esp_wifi_stop();
    }
}

static void wifi_task(void *arg)
{
    (void)arg;
    health_task_start();
    for (;;) {
        health_task_alive();

        wifi_req_t req;
        if (xQueueReceive(s_req_q, &req, 0) == pdTRUE) {
            if (req == WIFI_REQ_CONNECT) {
                begin_connect();
            } else {
                stop_wifi();
                s_state = WIFI_LINK_IDLE;
            }
        }

        if (s_state == WIFI_LINK_CONNECTING) {
            EventBits_t bits = xEventGroupGetBits(s_evt_group);
            uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
            if (bits & WIFI_CONNECTED_BIT) {
                s_state = WIFI_LINK_CONNECTED;
            } else if (bits & WIFI_FAIL_BIT) {
                stop_wifi();
                s_state = WIFI_LINK_FAILED;
            } else if ((uint32_t)(now_ms - s_connect_start_ms) >= CONFIG_WIFI_CONNECT_TIMEOUT_MS) {
                ESP_LOGW(TAG, "connect timeout");
                stop_wifi();
                s_state = WIFI_LINK_FAILED;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_TASK_PERIOD_MS));
    }
}

// --- Console: `wifi set/show/clear` (docs/ota-update-spec.md 3.2) ---

static int cmd_wifi(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "set") == 0 && argc == 4) {
        esp_err_t err = save_credentials(argv[2], argv[3]);
        if (err != ESP_OK) {
            printf("save failed: %s\n", esp_err_to_name(err));
            return 1;
        }
        printf("saved\n");
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "show") == 0 && argc == 2) {
        char ssid[33] = { 0 };
        char psk[65] = { 0 };
        if (load_credentials(ssid, sizeof(ssid), psk, sizeof(psk)) == ESP_OK) {
            printf("ssid: %s\n", ssid);
        } else {
            printf("no credentials stored\n");
        }
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "clear") == 0 && argc == 2) {
        esp_err_t err = clear_credentials();
        printf(err == ESP_OK ? "cleared\n" : "clear failed: %s\n", esp_err_to_name(err));
        return err == ESP_OK ? 0 : 1;
    }
    printf("usage: wifi set <ssid> <psk> | wifi show | wifi clear\n");
    return 1;
}

static void console_start(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "duster>";
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

    const esp_console_cmd_t cmd = {
        .command = "wifi",
        .help = "Wi-Fi credentials: wifi set <ssid> <psk> | wifi show | wifi clear",
        .hint = NULL,
        .func = &cmd_wifi,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

esp_err_t wifi_link_start(void)
{
    s_req_q = xQueueCreate(2, sizeof(wifi_req_t));
    s_evt_group = xEventGroupCreate();
    if (!s_req_q || !s_evt_group) {
        return ESP_ERR_NO_MEM;
    }

    console_start();

    BaseType_t ok = xTaskCreatePinnedToCore(wifi_task, "wifi", WIFI_TASK_STACK, NULL, WIFI_TASK_PRIO, NULL, WIFI_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

void wifi_link_request_connect(void)
{
    wifi_req_t req = WIFI_REQ_CONNECT;
    xQueueSend(s_req_q, &req, 0);
}

void wifi_link_request_disconnect(void)
{
    wifi_req_t req = WIFI_REQ_DISCONNECT;
    xQueueSend(s_req_q, &req, 0);
}

wifi_link_state_t wifi_link_get_state(void)
{
    return s_state;
}
