#include "obd_client.h"
#include "obd_core.h"
#include "can_link.h"
#include "control.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "obd";

#define OBD_TASK_CORE   1
#define OBD_TASK_PRIO   3
#define OBD_TASK_STACK  4096
#define OBD_LOOP_MS     10   // resolution for the 50 ms P2 timeout

#if !CONFIG_CAN_LISTEN_ONLY
static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static const char *status_name(dtc_status_t s)
{
    switch (s) {
    case DTC_NOT_READ:       return "not read";
    case DTC_READING:        return "reading";
    case DTC_NONE:           return "none";
    case DTC_PRESENT_NO_MIL: return "present, MIL off";
    case DTC_PRESENT_MIL:    return "present, MIL on";
    case DTC_READ_FAILED:    return "read failed";
    default:                 return "?";
    }
}

static void log_dtc(const dtc_state_t *d)
{
    char codes[8 * 6 + 1] = "";
    for (int i = 0; i < d->codes_read; i++) {
        strcat(codes, d->codes[i]);
        strcat(codes, " ");
    }
    ESP_LOGI(TAG, "DTC %s: count %u, MIL %d, codes read %u %s",
             status_name(d->status), d->count, d->mil_on, d->codes_read, codes);
}

static void send_outbox(obd_core_t *c)
{
    for (int i = 0; i < c->outbox_len; i++) {
        const obd_frame_t *f = &c->outbox[i];
        esp_err_t err = can_link_send(f->id, f->data);
        if (err == ESP_OK) {
            ESP_LOGD(TAG, "tx 0x%03lX %02X %02X %02X", (unsigned long)f->id, f->data[0], f->data[1], f->data[2]);
        } else {
            // Treated like no answer: the client retries on its own schedule.
            ESP_LOGW(TAG, "tx 0x%03lX failed: %s", (unsigned long)f->id, esp_err_to_name(err));
        }
    }
    c->outbox_len = 0;
}

static void obd_task(void *arg)
{
    (void)arg;
    const obd_cfg_t cfg = {
        .ignition_wait_ms = CONFIG_DTC_IGNITION_WAIT_MS,
        .retry_ms = CONFIG_DTC_RETRY_MS,
        .retry_count = CONFIG_DTC_RETRY_COUNT,
        .repoll_ms = CONFIG_DTC_REPOLL_S * 1000u,
        .p2_ms = CONFIG_OBD_P2_MS,
        .cf_timeout_ms = CONFIG_ISOTP_CF_TIMEOUT_MS,
    };
    static obd_core_t core;
    obd_core_init(&core, &cfg, now_ms());

    for (;;) {
        ui_model_t m;
        model_snapshot(&m);

        obd_frame_t rx = { .id = OBD_ID_ENGINE_RESP };
        bool got = can_link_obd_receive(rx.data, OBD_LOOP_MS);
        obd_core_step(&core, now_ms(), m.ignition_raw, got ? &rx : NULL);
        // Drain any further responses already queued (multi-frame Mode 03).
        while (can_link_obd_receive(rx.data, 0)) {
            send_outbox(&core);
            obd_core_step(&core, now_ms(), m.ignition_raw, &rx);
        }
        send_outbox(&core);

        if (core.dtc_changed) {
            core.dtc_changed = false;
            log_dtc(&core.dtc);
            control_obd_dtc(&core.dtc);
        }
    }
}

#endif

esp_err_t obd_client_start(void)
{
#if CONFIG_CAN_LISTEN_ONLY
    ESP_LOGI(TAG, "CAN listen-only: DTC reads disabled");
    return ESP_OK;
#else
    BaseType_t ok = xTaskCreatePinnedToCore(obd_task, "obd_client", OBD_TASK_STACK, NULL,
                                            OBD_TASK_PRIO, NULL, OBD_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
#endif
}
