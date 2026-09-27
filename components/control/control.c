#include "control.h"
#include "control_core.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "control";

#define CONTROL_TASK_CORE   1
#define CONTROL_TASK_PRIO   4
#define CONTROL_TASK_STACK  4096
#define CTRL_REQ_Q_DEPTH    8

static QueueHandle_t s_req_q;
static SemaphoreHandle_t s_lock;
static ui_model_t s_model;   // guarded by s_lock; critical sections are struct copies
static ctrl_core_t s_core;   // control task only
static uint16_t s_seq;

static const char *mode_name(op_mode_t m)
{
    return m == MODE_AUTO ? "AUTO" : "MANUAL";
}

static void publish(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ui_model_t *m = &s_model;
    m->mode = s_core.mode;
    m->manual_state = s_core.manual_state;
    m->aux_commanded = s_core.aux_commanded;
    m->last_applied_req_seq = s_core.last_applied_req_seq;
#if CONFIG_CONTROL_STUB_AUX_ACK
    // No aux_link yet (step 5): treat every command as ACKed at once.
    m->aux_actual = s_core.aux_commanded;
#endif
    m->frost_risk = ctrl_temp_flag(m->frost_risk, m->temp_stale, m->ext_temp_c,
                                   CONFIG_FROST_RISK_ON_C, CONFIG_TEMP_FLAG_HYSTERESIS_C);
    m->below_freezing = ctrl_temp_flag(m->below_freezing, m->temp_stale, m->ext_temp_c,
                                       CONFIG_BELOW_FREEZING_ON_C, CONFIG_TEMP_FLAG_HYSTERESIS_C);
    xSemaphoreGive(s_lock);
}

static void control_task(void *arg)
{
    (void)arg;
    ctrl_req_t req;
    for (;;) {
        // Wake on a request or at the control period, whichever comes first.
        if (xQueueReceive(s_req_q, &req, pdMS_TO_TICKS(CONFIG_CONTROL_PERIOD_MS)) == pdTRUE) {
            do {
                if (ctrl_core_apply(&s_core, &req)) {
                    ESP_LOGI(TAG, "req type %d seq %u applied, mode %s", req.type, req.seq, mode_name(s_core.mode));
                } else {
                    ESP_LOGW(TAG, "req type %d seq %u rejected in %s", req.type, req.seq, mode_name(s_core.mode));
                }
            } while (xQueueReceive(s_req_q, &req, 0) == pdTRUE);
        }

        // Vehicle light inputs arrive via can_lights_q from step 3; none yet.
        const vehicle_lights_t veh = { 0 };
        if (ctrl_core_update_commanded(&s_core, veh)) {
            ESP_LOGI(TAG, "commanded ledbar %d fog %d", s_core.aux_commanded.ledbar, s_core.aux_commanded.fog);
        }
        publish();
    }
}

esp_err_t control_start(void)
{
    s_req_q = xQueueCreate(CTRL_REQ_Q_DEPTH, sizeof(ctrl_req_t));
    s_lock = xSemaphoreCreateMutex();
    if (!s_req_q || !s_lock) {
        return ESP_ERR_NO_MEM;
    }

    ctrl_core_init(&s_core);
    s_model = (ui_model_t){
        .mode = MODE_MANUAL,
        .speed_stale = true,
        .temp_stale = true,
        .remote_link = REMOTE_LINK_NOT_CONFIGURED,
    };

    BaseType_t ok = xTaskCreatePinnedToCore(control_task, "control", CONTROL_TASK_STACK, NULL,
                                            CONTROL_TASK_PRIO, NULL, CONTROL_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool control_request(const ctrl_req_t *req)
{
    if (xQueueSend(s_req_q, req, 0) != pdTRUE) {
        ESP_LOGW(TAG, "ctrl_req_q full, dropped req type %d seq %u", req->type, req->seq);
        return false;
    }
    return true;
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
