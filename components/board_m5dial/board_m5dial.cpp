// M5Unified wrapper behind board_m5dial.h. The only file in the project that
// includes M5Unified / M5GFX (SPEC 3.3).

#include "board_m5dial.h"
#include "encoder.h"
#include "pins.h"

#include <M5Unified.hpp>
#include "esp_log.h"
#include "esp_check.h"
#include "soc/gpio_sig_map.h"

static const char *TAG = "board";

extern "C" void board_hold_on(void)
{
    gpio_reset_pin(PIN_HOLD);
    gpio_set_direction(PIN_HOLD, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_HOLD, 1);
}

extern "C" esp_err_t board_init(void)
{
    auto cfg = M5.config();
    // Only display, touch, button and hold are wanted (spike check 5).
    cfg.internal_imu = false;
    cfg.internal_rtc = false;
    cfg.internal_mic = false;
    cfg.internal_spk = false;
    // External devices probe Ex_I2C, which M5Unified maps to Port A
    // (G15/G13), owned by TWAI. Keep all of them off (spike check 3).
    cfg.external_imu = false;
    cfg.external_rtc = false;
    cfg.external_display_value = 0;
    cfg.external_speaker_value = 0;
    cfg.led_brightness = 0;
    M5.begin(cfg);

    if (M5.getBoard() != m5::board_t::board_M5Dial) {
        ESP_LOGE(TAG, "board detected as %d, not M5Dial", (int)M5.getBoard());
        return ESP_ERR_NOT_FOUND;
    }
    // Spike check 3. M5Unified records Port A as Ex_I2C (so Ex_I2C.isEnabled()
    // is true) but only claims the pins if the bus is begun. Check the pads
    // themselves: none may be driven by a peripheral or have output enabled.
    static const gpio_num_t owned[] = { PIN_CAN_TX, PIN_CAN_RX, PIN_AUX_UART_TX, PIN_AUX_UART_RX };
    for (gpio_num_t pin : owned) {
        gpio_io_config_t io = {};
        ESP_RETURN_ON_ERROR(gpio_get_io_config(pin, &io), TAG, "io config");
        if (io.oe || io.sig_out != SIG_GPIO_OUT_IDX) {
            ESP_LOGE(TAG, "GPIO%d claimed after M5.begin (oe %d, sig_out %lu)", pin, io.oe, (unsigned long)io.sig_out);
            return ESP_ERR_INVALID_STATE;
        }
    }
    ESP_LOGI(TAG, "M5Dial detected, display %dx%d", (int)M5.Display.width(), (int)M5.Display.height());

    return encoder_init(PIN_ENC_A, PIN_ENC_B);
}

extern "C" void board_update(void)
{
    M5.update();
}

extern "C" void board_display_flush(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *px)
{
    // As M5Dial-LVGL lv_port_disp.cpp: LVGL RGB565 is little-endian, swap on push.
    M5.Display.setAddrWindow(x, y, w, h);
    M5.Display.pushPixels(px, w * h, true);
    M5.Display.waitDisplay();
}

extern "C" void board_display_brightness(uint8_t level)
{
    M5.Display.setBrightness(level);
}

extern "C" bool board_touch_read(int32_t *x, int32_t *y)
{
    if (M5.Touch.getCount() == 0) {
        return false;
    }
    auto detail = M5.Touch.getDetail();
    *x = detail.x;
    *y = detail.y;
    return true;
}

extern "C" bool board_button_pressed(void)
{
    return M5.BtnA.isPressed();
}

extern "C" int32_t board_encoder_take(void)
{
    return encoder_take();
}
