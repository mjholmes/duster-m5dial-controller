// TWAI on Grove Port A (SPEC 3.2, 5.1). Uses the ESP-IDF 5.5 esp_twai on-chip
// driver: frames can only be read inside the receive callback, so the callback
// copies each one into a queue and can_rx blocks on that queue.

#include "can_link.h"
#include "can_signals.h"
#include "control.h"
#include "pins.h"
#include "health.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "can";

#define CAN_BITRATE        500000
#define CAN_RX_Q_DEPTH     128   // ~100 ms of frames at the measured 1300 frames/s
#define CAN_RX_TASK_CORE   0
#define CAN_RX_TASK_PRIO   6
#define CAN_RX_TASK_STACK  4096
#define STATS_PERIOD_MS    5000
#define SEEN_IDS_MAX       64
#define OBD_RX_Q_DEPTH     8
#define TX_SLOTS           4     // = tx_queue_depth
// Standard CAN automatic retransmission. On the ESP32-S3 (TWAI HAL v1) any
// value other than -1 selects single-shot mode (hal/twai_hal_v1.c:
// ".ss = retry_cnt != -1"), so a single lost arbitration dropped the frame.
#define TX_RETRIES         -1

#if CONFIG_CAN_LISTEN_ONLY
#define LISTEN_ONLY 1
#else
#define LISTEN_ONLY 0
#endif

typedef struct {
    uint32_t id;
    uint8_t  len;
    uint8_t  flags;      // bit 0 extended, bit 1 RTR
    uint8_t  data[8];
} can_rx_frame_t;

static twai_node_handle_t s_node;
static QueueHandle_t s_rx_q;
static QueueHandle_t s_obd_rx_q;

// The driver keeps a pointer to each frame until it is sent, so frames live
// in static slots freed by on_tx_done.
static struct {
    twai_frame_t  frame;
    uint8_t       data[8];
    volatile bool busy;
} s_tx[TX_SLOTS];
#if !CONFIG_CAN_LISTEN_ONLY
static portMUX_TYPE s_tx_lock = portMUX_INITIALIZER_UNLOCKED;
#endif
static volatile uint32_t s_tx_failed;
static volatile bool s_bus_off;
static volatile uint32_t s_isr_dropped;
static volatile int s_bus_state = -1;   // twai_error_state_t, -1 = no change pending

// Diagnostics, can_rx task only.
static struct {
    uint32_t frames;
    uint32_t count[CAN_SIG_COUNT];
    uint32_t malformed[CAN_SIG_COUNT];
    uint32_t unknown_value[CAN_SIG_COUNT];
    uint32_t obd;
    uint32_t seen_ids[SEEN_IDS_MAX];
    int      seen_count;
} s_stats;

static bool IRAM_ATTR on_rx_done(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    (void)edata;
    (void)user_ctx;
    can_rx_frame_t f;
    twai_frame_t rx = {
        .buffer = f.data,
        .buffer_len = sizeof(f.data),
    };
    if (twai_node_receive_from_isr(handle, &rx) != ESP_OK) {
        return false;
    }
    f.id = rx.header.id;
    f.len = rx.header.dlc > sizeof(f.data) ? sizeof(f.data) : (uint8_t)rx.header.dlc;
    f.flags = (rx.header.ide ? 1 : 0) | (rx.header.rtr ? 2 : 0);

    BaseType_t woken = pdFALSE;
    if (xQueueSendFromISR(s_rx_q, &f, &woken) != pdTRUE) {
        s_isr_dropped++;
    }
    return woken == pdTRUE;
}

static bool IRAM_ATTR on_tx_done(twai_node_handle_t handle, const twai_tx_done_event_data_t *edata, void *user_ctx)
{
    (void)handle;
    (void)user_ctx;
    if (!edata->is_tx_success) {
        s_tx_failed++;
    }
    for (int i = 0; i < TX_SLOTS; i++) {
        if (edata->done_tx_frame == &s_tx[i].frame) {
            s_tx[i].busy = false;
        }
    }
    return false;
}

static bool IRAM_ATTR on_state_change(twai_node_handle_t handle, const twai_state_change_event_data_t *edata, void *user_ctx)
{
    (void)handle;
    (void)user_ctx;
    s_bus_state = edata->new_sta;
    s_bus_off = edata->new_sta == TWAI_ERROR_BUS_OFF;
    return false;
}

#if CONFIG_CAN_LOG_STATS
static void note_id(uint32_t id)
{
    for (int i = 0; i < s_stats.seen_count; i++) {
        if (s_stats.seen_ids[i] == id) {
            return;
        }
    }
    if (s_stats.seen_count < SEEN_IDS_MAX) {
        s_stats.seen_ids[s_stats.seen_count++] = id;
    }
}
#endif

static const char *bus_state_name(int state)
{
    switch (state) {
    case TWAI_ERROR_ACTIVE:  return "error active (ok)";
    case TWAI_ERROR_WARNING: return "error warning";
    case TWAI_ERROR_PASSIVE: return "error passive";
    case TWAI_ERROR_BUS_OFF: return "bus off";
    default:                 return "?";
    }
}

static void handle_frame(const can_rx_frame_t *f, uint32_t now)
{
    static vehicle_lights_t last_lights;
    static bool have_lights;

    s_stats.frames++;
#if CONFIG_CAN_LOG_STATS
    note_id(f->id);
    if (CONFIG_CAN_DUMP_ID != 0 && f->id == CONFIG_CAN_DUMP_ID) {
        static uint8_t last[8];
        static uint8_t last_len = 0xFF;
        if (f->len != last_len || memcmp(f->data, last, f->len) != 0) {
            memcpy(last, f->data, f->len);
            last_len = f->len;
            ESP_LOGI(TAG, "dump 0x%03lX [%u] %02X %02X %02X %02X %02X %02X %02X %02X", (unsigned long)f->id, f->len,
                     f->data[0], f->data[1], f->data[2], f->data[3], f->data[4], f->data[5], f->data[6], f->data[7]);
        }
    }
#endif
    if (can_is_obd_response(f->id)) {
        s_stats.obd++;
        if (f->len == 8 && xQueueSend(s_obd_rx_q, f->data, 0) != pdTRUE) {
            ESP_LOGW(TAG, "obd_rx_q full, dropped 0x7E8 frame");   // SPEC 5: drop, log
        }
        return;
    }

    can_decoded_t d;
    can_dec_result_t r = can_decode(f->id, f->flags & 1, f->flags & 2, f->data, f->len, &d);
    if (r == CAN_DEC_NOT_SUBSCRIBED) {
        return;
    }
    if (r == CAN_DEC_MALFORMED) {
        s_stats.malformed[d.sig]++;
        return;
    }
    s_stats.count[d.sig]++;
    if (r == CAN_DEC_UNKNOWN_VALUE) {
        s_stats.unknown_value[d.sig]++;
#if CONFIG_CAN_LOG_STATS
        // Log each distinct unknown ignition value once.
        static uint32_t logged[8];
        if (d.sig == CAN_SIG_IGNITION && !(logged[d.ignition_raw >> 5] & (1u << (d.ignition_raw & 31)))) {
            logged[d.ignition_raw >> 5] |= 1u << (d.ignition_raw & 31);
            ESP_LOGW(TAG, "ignition: unknown value 0x%02X", d.ignition_raw);
        }
#endif
        return;
    }

    switch (d.sig) {
    case CAN_SIG_SPEED:
        control_can_speed(d.speed.mph, now);
        break;
    case CAN_SIG_TEMP:
        control_can_temp(d.ext_temp_c, now);
        break;
    case CAN_SIG_IGNITION:
        control_can_ignition(d.ignition_raw);
        break;
    case CAN_SIG_LIGHTS: {
        bool changed = !have_lights || memcmp(&d.lights, &last_lights, sizeof(last_lights)) != 0;
        if (changed) {
            ESP_LOGI(TAG, "lights: main %d dipped %d fog %d", d.lights.main_beam, d.lights.dipped_beam, d.lights.front_fog);
        }
        last_lights = d.lights;
        have_lights = true;
        control_can_lights(d.lights, changed, now);
        break;
    }
    default:
        break;
    }
}

#if CONFIG_CAN_LOG_STATS
static void log_stats(void)
{
    ui_model_t m;
    model_snapshot(&m);
    twai_node_status_t st = { 0 };
    twai_node_record_t rec = { 0 };
    twai_node_get_info(s_node, &st, &rec);
    static uint32_t last_bus_err;
    ESP_LOGI(TAG, "%lu frames, %d IDs seen, isr dropped %lu, obd %lu, tx failed %lu, %s (rec %u tec %u), bus errors +%lu",
             (unsigned long)s_stats.frames, s_stats.seen_count,
             (unsigned long)s_isr_dropped, (unsigned long)s_stats.obd, (unsigned long)s_tx_failed,
             bus_state_name(st.state), st.rx_error_count, st.tx_error_count,
             (unsigned long)(rec.bus_err_num - last_bus_err));
    last_bus_err = rec.bus_err_num;
    for (int i = 0; i < CAN_SIG_COUNT; i++) {
        ESP_LOGI(TAG, "  0x%03lX %-15s rx %lu malformed %lu unknown %lu",
                 (unsigned long)can_sig_id(i), can_sig_name(i), (unsigned long)s_stats.count[i],
                 (unsigned long)s_stats.malformed[i], (unsigned long)s_stats.unknown_value[i]);
    }
    ESP_LOGI(TAG, "  speed %u mph%s, temp %d C%s, ignition 0x%02X, lights %s",
             m.speed_mph, m.speed_stale ? " (stale)" : "", m.ext_temp_c, m.temp_stale ? " (stale)" : "",
             m.ignition_raw, m.can_timeout ? "timeout" : "ok");
    // Every distinct ID seen, logged when the set grows (confirms bitrate/pins).
    static int logged_ids;
    if (s_stats.seen_count != logged_ids) {
        logged_ids = s_stats.seen_count;
        char line[8 * SEEN_IDS_MAX];
        int n = 0;
        for (int i = 0; i < s_stats.seen_count && n < (int)sizeof(line) - 6; i++) {
            n += snprintf(line + n, sizeof(line) - n, "%03lX ", (unsigned long)s_stats.seen_ids[i]);
        }
        ESP_LOGI(TAG, "  IDs: %s", line);
    }
    memset(s_stats.count, 0, sizeof(s_stats.count));
    s_stats.frames = 0;
}
#endif

static void can_rx_task(void *arg)
{
    (void)arg;
    uint32_t last_stats = (uint32_t)(esp_timer_get_time() / 1000);
    uint32_t bus_off_since = 0;
    bool recovering = false;
    can_rx_frame_t f;
    health_task_start();
    for (;;) {
        health_task_alive();
        // Bounded wait so the task can feed the watchdog (step 7) and log.
        if (xQueueReceive(s_rx_q, &f, pdMS_TO_TICKS(CONFIG_QUEUE_WAIT_MS)) == pdTRUE) {
            handle_frame(&f, (uint32_t)(esp_timer_get_time() / 1000));
        }

        uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
        int state = s_bus_state;
        if (state >= 0) {
            s_bus_state = -1;
            ESP_LOGW(TAG, "bus state: %s", bus_state_name(state));
            control_can_bus_off(state == TWAI_ERROR_BUS_OFF);
            if (state == TWAI_ERROR_BUS_OFF) {
                bus_off_since = now_ms;
                recovering = false;
            }
        }
        // Bus-off recovery with a back-off, never in a tight loop (SPEC 8.5).
        // The controller rejoins after 129 x 11 recessive bits; on_state_change
        // then reports error active.
        if (s_bus_off && !recovering && (uint32_t)(now_ms - bus_off_since) >= CONFIG_CAN_BUS_OFF_BACKOFF_MS) {
            ESP_LOGW(TAG, "bus off for %d ms, recovering", CONFIG_CAN_BUS_OFF_BACKOFF_MS);
            recovering = twai_node_recover(s_node) == ESP_OK;
            if (!recovering) {
                bus_off_since = now_ms;   // try again after another back-off
            }
        }

#if CONFIG_CAN_LOG_STATS
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if ((uint32_t)(now - last_stats) >= STATS_PERIOD_MS) {
            last_stats = now;
            log_stats();
        }
#else
        (void)last_stats;
#endif
    }
}

esp_err_t can_link_start(void)
{
    s_rx_q = xQueueCreate(CAN_RX_Q_DEPTH, sizeof(can_rx_frame_t));
    s_obd_rx_q = xQueueCreate(OBD_RX_Q_DEPTH, 8);
    if (!s_rx_q || !s_obd_rx_q) {
        return ESP_ERR_NO_MEM;
    }

    twai_onchip_node_config_t cfg = {
        .io_cfg = {
            .tx = PIN_CAN_TX,
            .rx = PIN_CAN_RX,
            .quanta_clk_out = -1,
            .bus_off_indicator = -1,
        },
        .bit_timing = { .bitrate = CAN_BITRATE },
        .tx_queue_depth = TX_SLOTS,
        .fail_retry_cnt = TX_RETRIES,
        .flags = {
            .enable_listen_only = LISTEN_ONLY,
        },
    };
    ESP_ERROR_CHECK(twai_new_node_onchip(&cfg, &s_node));

    const twai_event_callbacks_t cbs = {
        .on_rx_done = on_rx_done,
        .on_tx_done = on_tx_done,
        .on_state_change = on_state_change,
    };
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(s_node, &cbs, NULL));

    BaseType_t ok = xTaskCreatePinnedToCore(can_rx_task, "can_rx", CAN_RX_TASK_STACK, NULL,
                                            CAN_RX_TASK_PRIO, NULL, CAN_RX_TASK_CORE);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    ESP_ERROR_CHECK(twai_node_enable(s_node));
    ESP_LOGI(TAG, "TWAI %d kbps, %s, TX G%d RX G%d", CAN_BITRATE / 1000,
             LISTEN_ONLY ? "listen-only" : "normal", PIN_CAN_TX, PIN_CAN_RX);
    return ESP_OK;
}

esp_err_t can_link_send(uint32_t id, const uint8_t data[8])
{
#if CONFIG_CAN_LISTEN_ONLY
    (void)id;
    (void)data;
    return ESP_ERR_NOT_SUPPORTED;
#else
    if (s_bus_off) {
        return ESP_ERR_INVALID_STATE;
    }
    int slot = -1;
    taskENTER_CRITICAL(&s_tx_lock);
    for (int i = 0; i < TX_SLOTS; i++) {
        if (!s_tx[i].busy) {
            s_tx[i].busy = true;
            slot = i;
            break;
        }
    }
    taskEXIT_CRITICAL(&s_tx_lock);
    if (slot < 0) {
        return ESP_ERR_TIMEOUT;
    }
    memcpy(s_tx[slot].data, data, 8);
    s_tx[slot].frame = (twai_frame_t){
        .header = { .id = id, .dlc = 8 },
        .buffer = s_tx[slot].data,
        .buffer_len = 8,
    };
    esp_err_t err = twai_node_transmit(s_node, &s_tx[slot].frame, 0);
    if (err != ESP_OK) {
        s_tx[slot].busy = false;
    }
    return err;
#endif
}

bool can_link_obd_receive(uint8_t data[8], uint32_t wait_ms)
{
    return xQueueReceive(s_obd_rx_q, data, pdMS_TO_TICKS(wait_ms)) == pdTRUE;
}
