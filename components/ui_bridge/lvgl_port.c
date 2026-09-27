// LVGL 9 port: display on board_m5dial, touch as pointer, encoder plus front
// button as encoder device, tick from esp_timer (SPEC 5 note 1, 9.3).
// Called only from the ui task.

#include "lvgl_port.h"
#include "board_m5dial.h"

#include "esp_timer.h"
#include "esp_check.h"
#include "esp_log.h"

#define TICK_PERIOD_MS 2
#define BUF_ROWS       24  // partial buffers; no PSRAM on the M5Dial

static lv_color_t s_buf1[BOARD_LCD_WIDTH * BUF_ROWS];
static lv_color_t s_buf2[BOARD_LCD_WIDTH * BUF_ROWS];

static lv_indev_t *s_encoder;
#if CONFIG_UI_BRINGUP_DIAG
static uint32_t s_frames;
static uint32_t s_flushed_px;
static int64_t s_fps_window_us;

// Bring-up: frame rate for spike check 2 (SPEC 3.3), logged every 5 s.
static void refr_ready_cb(lv_event_t *e)
{
    (void)e;
    s_frames++;
    int64_t now = esp_timer_get_time();
    if (now - s_fps_window_us >= 5000000) {
        ESP_LOGI("lvgl", "frames %lu in %lld ms", (unsigned long)s_frames, (now - s_fps_window_us) / 1000);
        s_frames = 0;
        s_fps_window_us = now;
    }
}
#endif

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
#if CONFIG_UI_BRINGUP_DIAG
    s_flushed_px += (uint32_t)lv_area_get_size(area);
#endif
    board_display_flush(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area),
                        (const uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    static int32_t last_x, last_y;
    (void)indev;
    bool pressed = board_touch_read(&last_x, &last_y);
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
#if CONFIG_UI_BRINGUP_DIAG
    static bool was_pressed;
    if (pressed != was_pressed) {
        ESP_LOGI("lvgl", "touch %s at %ld,%ld", pressed ? "down" : "up", (long)last_x, (long)last_y);
        was_pressed = pressed;
    }
#endif
    data->point.x = last_x;
    data->point.y = last_y;
}

static void encoder_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    data->enc_diff = (int16_t)board_encoder_take();
#if CONFIG_UI_BRINGUP_DIAG
    if (data->enc_diff) {
        ESP_LOGI("lvgl", "encoder %+d", data->enc_diff);
    }
#endif
    // The front button bypasses LVGL (SPEC 9.3), so the encoder has no enter key.
    data->state = LV_INDEV_STATE_RELEASED;
}

static void tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(TICK_PERIOD_MS);
}

esp_err_t lvgl_port_init(void)
{
    lv_init();

    lv_display_t *disp = lv_display_create(BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, s_buf1, s_buf2, sizeof(s_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#if CONFIG_UI_BRINGUP_DIAG
    lv_display_add_event_cb(disp, refr_ready_cb, LV_EVENT_REFR_READY, NULL);
#endif

    lv_indev_t *touch = lv_indev_create();
    lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch, touch_read_cb);

    s_encoder = lv_indev_create();
    lv_indev_set_type(s_encoder, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(s_encoder, encoder_read_cb);

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(s_encoder, group);

    const esp_timer_create_args_t args = {
        .callback = tick_cb,
        .name = "lv_tick",
    };
    esp_timer_handle_t timer;
    ESP_RETURN_ON_ERROR(esp_timer_create(&args, &timer), "lvgl", "tick timer");
    return esp_timer_start_periodic(timer, TICK_PERIOD_MS * 1000);
}

#if CONFIG_UI_BRINGUP_DIAG
void lvgl_port_benchmark(void)
{
    const int frames = 30;
    // Let the initial screen fade finish so the whole screen is really redrawn.
    int64_t settle = esp_timer_get_time() + 400000;
    while (esp_timer_get_time() < settle) {
        lv_timer_handler();
    }
    lv_obj_t *scr = lv_screen_active();
    lv_refr_now(NULL);
    s_flushed_px = 0;
    int64_t start = esp_timer_get_time();
    for (int i = 0; i < frames; i++) {
        lv_obj_invalidate(scr);
        lv_refr_now(NULL);
    }
    int64_t us = esp_timer_get_time() - start;
    ESP_LOGI("lvgl", "benchmark: %d full redraws in %lld ms, %.1f frames/s, %lu px flushed (expect %d)",
             frames, us / 1000, frames * 1e6 / (double)us, (unsigned long)s_flushed_px,
             frames * BOARD_LCD_WIDTH * BOARD_LCD_HEIGHT);
}
#endif
