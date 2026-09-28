// ui task: the only task that calls LVGL (SPEC 5 note 1).

#include "ui_bridge.h"
#include "lvgl_port.h"
#include "ui_vars.h"
#include "button_fsm.h"
#include "brightness.h"
#include "board_m5dial.h"
#include "control.h"
#include "settings.h"
#include "ui.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "ui";

#define UI_TASK_CORE     1
#define UI_TASK_PRIO     2
#define UI_TASK_STACK    6144

// Front button bypasses LVGL (SPEC 9.3, decided): short press = manual, all
// off; long press = AUTO.
static void handle_button(button_fsm_t *btn, uint32_t now_ms)
{
    btn_evt_t evt = button_fsm_step(btn, board_button_pressed(), now_ms, CONFIG_BUTTON_LONG_PRESS_MS);
    if (evt == BTN_EVT_NONE) {
        return;
    }
    ctrl_req_t req = {
        .src = REQ_SRC_LOCAL_UI,
        .seq = control_next_seq(),
    };
    if (evt == BTN_EVT_SHORT) {
        req.type = REQ_MANUAL_ALL_OFF;
    } else {
        req.type = REQ_SET_MODE;
        req.mode = MODE_AUTO;
    }
    control_request(&req);
}

// Encoder sets display brightness (decided; replaces LVGL focus navigation).
// The setting is saved; night dim is applied on top while headlights are on.
static void handle_brightness(const ui_model_t *m, int *shown_pct)
{
    static const brightness_cfg_t cfg = {
        .min_pct = CONFIG_DISPLAY_BRIGHTNESS_MIN_PCT,
        .max_pct = 100,
        .step_pct = CONFIG_DISPLAY_BRIGHTNESS_STEP_PCT,
    };
    int32_t detents = board_encoder_take();
    if (detents != 0) {
        settings_set_brightness_pct(brightness_step(settings_brightness_pct(), detents, &cfg));
    }

    bool night = !m->can_timeout && (m->veh.dipped_beam || m->veh.main_beam);
    int pct = brightness_effective(settings_brightness_pct(), night,
                                   CONFIG_DISPLAY_NIGHT_DIM_PCT, CONFIG_DISPLAY_BRIGHTNESS_MIN_PCT);
    if (pct != *shown_pct) {
        *shown_pct = pct;
        board_display_brightness(brightness_to_level(pct));
    }
}

static void ui_task(void *arg)
{
    (void)arg;
    ESP_ERROR_CHECK(lvgl_port_init());
    ui_vars_refresh((uint32_t)(esp_timer_get_time() / 1000));
    ui_init();
    ESP_LOGI(TAG, "LVGL %d.%d.%d up", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
#if CONFIG_UI_BRINGUP_DIAG
    lvgl_port_benchmark();
#endif

    int shown_pct = -1;

    button_fsm_t btn = { 0 };
    for (;;) {
        uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
        board_update();
        handle_button(&btn, now_ms);

        ui_vars_refresh(now_ms);
        handle_brightness(ui_vars_model(), &shown_pct);
        settings_service(now_ms);
        ui_tick();
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(CONFIG_UI_LOOP_MS));
    }
}

esp_err_t ui_bridge_start(void)
{
    BaseType_t ok = xTaskCreatePinnedToCore(ui_task, "ui", UI_TASK_STACK, NULL, UI_TASK_PRIO, NULL, UI_TASK_CORE);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}
