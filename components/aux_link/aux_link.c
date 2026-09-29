// aux task: sole owner of auxbeam-light-controller (SPEC 7.2). It calls
// auxbeam_light_init() itself; no other task may call auxbeam_light_*().

#include "aux_link.h"
#include "aux_sm.h"
#include "control.h"
#include "pins.h"
#include "health.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "aux";

#define AUX_TASK_CORE      0
#define AUX_TASK_PRIO      5
#define AUX_TASK_STACK     4096
#define AUX_UART           UART_NUM_1
#define INIT_RETRY_MS      1000

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static auxbeam_context_t *init_library(void)
{
    const auxbeam_config_t cfg = {
        .uart_num = AUX_UART,
        .tx_pin = PIN_AUX_UART_TX,
        .rx_pin = PIN_AUX_UART_RX,
        .default_timeout_ms = CONFIG_AUX_ACK_TIMEOUT_MS,
    };
    for (;;) {
        auxbeam_context_t *ctx = NULL;
        auxbeam_error_t err = auxbeam_light_init(&cfg, &ctx);
        if (err == AUXBEAM_OK) {
            ESP_LOGI(TAG, "RA60 link on UART%d, TX G%d RX G%d, ACK timeout %d ms, outputs %d/%d",
                     AUX_UART, PIN_AUX_UART_TX, PIN_AUX_UART_RX, CONFIG_AUX_ACK_TIMEOUT_MS,
                     CONFIG_AUX_CH_LEDBAR + 1, CONFIG_AUX_CH_FOG + 1);
            return ctx;
        }
        // The link reads as lost in control until this succeeds.
        ESP_LOGE(TAG, "auxbeam_light_init failed (%d), retrying", err);
        health_task_alive();
        vTaskDelay(pdMS_TO_TICKS(INIT_RETRY_MS));
    }
}

static void aux_task(void *arg)
{
    (void)arg;
#if CONFIG_AUX_LOG_FRAMES
    esp_log_level_set("auxbeam_light", ESP_LOG_DEBUG);
#endif
    health_task_start();
    auxbeam_context_t *ctx = init_library();

    const aux_sm_cfg_t cfg = {
        .ch_ledbar = CONFIG_AUX_CH_LEDBAR,
        .ch_fog = CONFIG_AUX_CH_FOG,
        .keepalive_ms = CONFIG_AUX_KEEPALIVE_MS,
    };
    static aux_sm_t sm;
    aux_sm_init(&sm, &cfg, ctx);

    bool failing = false;
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        health_task_alive();
        aux_lights_t target;
        if (control_aux_take_target(&target)) {
            aux_sm_set_target(&sm, target);
        }

        uint32_t now = now_ms();
        switch (aux_sm_poll(&sm, now)) {
        case AUX_SM_OUT_ACKED:
            if (failing) {
                ESP_LOGI(TAG, "RA60 acknowledging again");
                failing = false;
            }
            control_aux_event(true, sm.acked, now);
            break;
        case AUX_SM_OUT_FAILED:
            // Logged once per outage; the retry/back-off cycle repeats quietly.
            if (!failing) {
                ESP_LOGW(TAG, "no ACK after retry; retrying every %d ms until the RA60 answers",
                         CONFIG_AUX_KEEPALIVE_MS + 2 * CONFIG_AUX_ACK_TIMEOUT_MS);
                failing = true;
            }
            control_aux_event(false, sm.acked, now);
            break;
        default:
            break;
        }
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(CONFIG_AUX_POLL_MS));
    }
}

esp_err_t aux_link_start(void)
{
    BaseType_t ok = xTaskCreatePinnedToCore(aux_task, "aux", AUX_TASK_STACK, NULL,
                                            AUX_TASK_PRIO, NULL, AUX_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}
