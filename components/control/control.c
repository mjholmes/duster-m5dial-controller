#include "control.h"
#include "control_core.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "control";

#define CONTROL_TASK_CORE   1
#define CONTROL_TASK_PRIO   4
#define CONTROL_TASK_STACK  4096
#define CTRL_REQ_Q_DEPTH    8
#define CAN_LIGHTS_Q_DEPTH  8

typedef struct {
    vehicle_lights_t lights;
    uint32_t rx_ms;
} can_lights_item_t;

static QueueHandle_t s_req_q;
static QueueHandle_t s_lights_q;
static TaskHandle_t s_task;
static SemaphoreHandle_t s_lock;

// Guarded by s_lock; critical sections are struct copies and field writes.
static ui_model_t s_model;
static uint32_t s_speed_rx_ms;
static uint32_t s_temp_rx_ms;
static uint32_t s_lights_rx_ms;

// control task only.
static ctrl_core_t s_core;
static vehicle_lights_t s_veh;

static uint16_t s_seq;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static bool older_than(uint32_t now, uint32_t then, uint32_t limit_ms)
{
    return (uint32_t)(now - then) >= limit_ms;
}

static const char *mode_name(op_mode_t m)
{
    return m == MODE_AUTO ? "AUTO" : "MANUAL";
}

static void publish(uint32_t now)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ui_model_t *m = &s_model;
    m->mode = s_core.mode;
    m->manual_state = s_core.manual_state;
    m->aux_commanded = s_core.aux_commanded;
    m->last_applied_req_seq = s_core.last_applied_req_seq;
    m->veh = s_veh;
#if CONFIG_CONTROL_STUB_AUX_ACK
    // No aux_link yet (step 5): treat every command as ACKed at once.
    m->aux_actual = s_core.aux_commanded;
#endif
    // Link health flags. The safe-state action on CAN timeout in AUTO (force
    // MANUAL, all off) and the link table come in step 4.
    m->speed_stale = older_than(now, s_speed_rx_ms, CONFIG_STALE_DISPLAY_MS);
    m->temp_stale = older_than(now, s_temp_rx_ms, CONFIG_STALE_DISPLAY_MS);
    m->can_timeout = older_than(now, s_lights_rx_ms, CONFIG_CAN_LIGHTS_TIMEOUT_MS);

    m->frost_risk = ctrl_temp_flag(m->frost_risk, m->temp_stale, m->ext_temp_c,
                                   CONFIG_FROST_RISK_ON_C, CONFIG_TEMP_FLAG_HYSTERESIS_C);
    m->below_freezing = ctrl_temp_flag(m->below_freezing, m->temp_stale, m->ext_temp_c,
                                       CONFIG_BELOW_FREEZING_ON_C, CONFIG_TEMP_FLAG_HYSTERESIS_C);
    xSemaphoreGive(s_lock);
}

static void update_commanded(void)
{
    if (ctrl_core_update_commanded(&s_core, s_veh)) {
        // Posted to aux_cmd_q from step 5.
        ESP_LOGI(TAG, "commanded ledbar %d fog %d", s_core.aux_commanded.ledbar, s_core.aux_commanded.fog);
    }
}

static void apply_request(const ctrl_req_t *req)
{
    if (ctrl_core_apply(&s_core, req)) {
        ESP_LOGI(TAG, "req type %d seq %u applied, mode %s", req->type, req->seq, mode_name(s_core.mode));
    } else {
        ESP_LOGW(TAG, "req type %d seq %u rejected in %s", req->type, req->seq, mode_name(s_core.mode));
    }
    update_commanded();
}

static void control_task(void *arg)
{
    (void)arg;
    const TickType_t period = pdMS_TO_TICKS(CONFIG_CONTROL_PERIOD_MS);
    TickType_t next_period = xTaskGetTickCount() + period;

    for (;;) {
        // Wake when a producer notifies (request or light-input change) or at
        // the control period, whichever comes first.
        TickType_t now_ticks = xTaskGetTickCount();
        int32_t until = (int32_t)(next_period - now_ticks);
        ulTaskNotifyTake(pdTRUE, until > 0 ? (TickType_t)until : 0);

        ctrl_req_t req;
        while (xQueueReceive(s_req_q, &req, 0) == pdTRUE) {
            apply_request(&req);
        }
        can_lights_item_t item;
        while (xQueueReceive(s_lights_q, &item, 0) == pdTRUE) {
            // Each edge is applied in order, so a short main-beam flash
            // reaches aux_commanded even if it is shorter than a period.
            s_veh = item.lights;
            update_commanded();
        }

        if ((int32_t)(xTaskGetTickCount() - next_period) >= 0) {
            next_period += period;
            update_commanded();
        }
        publish(now_ms());
    }
}

esp_err_t control_start(void)
{
    s_req_q = xQueueCreate(CTRL_REQ_Q_DEPTH, sizeof(ctrl_req_t));
    s_lights_q = xQueueCreate(CAN_LIGHTS_Q_DEPTH, sizeof(can_lights_item_t));
    s_lock = xSemaphoreCreateMutex();
    if (!s_req_q || !s_lights_q || !s_lock) {
        return ESP_ERR_NO_MEM;
    }

    ctrl_core_init(&s_core);
    // Timestamps start at boot, so every CAN signal reads as stale until its
    // first frame arrives.
    uint32_t now = now_ms();
    s_speed_rx_ms = s_temp_rx_ms = s_lights_rx_ms = now;
    s_model = (ui_model_t){
        .mode = MODE_MANUAL,
        .speed_stale = true,
        .temp_stale = true,
        .remote_link = REMOTE_LINK_NOT_CONFIGURED,
    };

    BaseType_t ok = xTaskCreatePinnedToCore(control_task, "control", CONTROL_TASK_STACK, NULL,
                                            CONTROL_TASK_PRIO, &s_task, CONTROL_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool control_request(const ctrl_req_t *req)
{
    if (xQueueSend(s_req_q, req, 0) != pdTRUE) {
        ESP_LOGW(TAG, "ctrl_req_q full, dropped req type %d seq %u", req->type, req->seq);
        return false;
    }
    xTaskNotifyGive(s_task);
    return true;
}

void control_can_speed(uint8_t mph, uint32_t rx_ms)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_model.speed_mph = mph;
    s_speed_rx_ms = rx_ms;
    xSemaphoreGive(s_lock);
}

void control_can_temp(int8_t temp_c, uint32_t rx_ms)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_model.ext_temp_c = temp_c;
    s_temp_rx_ms = rx_ms;
    xSemaphoreGive(s_lock);
}

void control_can_ignition(uint8_t raw)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_model.ignition_raw = raw;
    xSemaphoreGive(s_lock);
}

void control_can_lights(vehicle_lights_t lights, bool changed, uint32_t rx_ms)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_lights_rx_ms = rx_ms;
    xSemaphoreGive(s_lock);

    if (!changed) {
        return;
    }
    // Full policy: overwrite oldest (SPEC 5). Only can_rx sends to this queue.
    can_lights_item_t item = { .lights = lights, .rx_ms = rx_ms };
    if (xQueueSend(s_lights_q, &item, 0) != pdTRUE) {
        can_lights_item_t dropped;
        xQueueReceive(s_lights_q, &dropped, 0);
        xQueueSend(s_lights_q, &item, 0);
        ESP_LOGW(TAG, "can_lights_q full, dropped oldest");
    }
    xTaskNotifyGive(s_task);
}

void control_can_bus_off(bool bus_off)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_model.can_bus_off = bus_off;
    xSemaphoreGive(s_lock);
}

void model_snapshot(ui_model_t *out)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out = s_model;
    xSemaphoreGive(s_lock);
}

uint16_t control_next_seq(void)
{
    // Callers are the ui task only in Phase 1.
    return ++s_seq;
}
