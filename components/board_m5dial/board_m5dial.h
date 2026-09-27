// M5Dial board support: C API over M5Unified (display, touch, front button,
// hold) plus the PCNT encoder ported from the POC. SPEC 3.3.
//
// Threading: board_init() once from app_main. Every other call is made only
// from the ui task (the task that owns LVGL), except board_hold_on().
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_LCD_WIDTH  240
#define BOARD_LCD_HEIGHT 240

// Drive the hold pin high. Call first thing in app_main.
void board_hold_on(void);

// Bring up display, touch and button (M5Unified), and the encoder.
esp_err_t board_init(void);

// Refresh M5Unified input state (touch, button). ui task only.
void board_update(void);

// Push an RGB565 area (LVGL native byte order) to the LCD. Blocks until sent.
void board_display_flush(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *px);

void board_display_brightness(uint8_t level);

// Latest touch point; false when not touched. Valid after board_update().
bool board_touch_read(int32_t *x, int32_t *y);

// Front button state. Valid after board_update().
bool board_button_pressed(void);

// Encoder counts since the previous call (positive = clockwise).
int32_t board_encoder_take(void);

#ifdef __cplusplus
}
#endif
