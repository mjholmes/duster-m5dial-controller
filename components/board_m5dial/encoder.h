// Internal to board_m5dial.
#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t encoder_init(gpio_num_t pin_a, gpio_num_t pin_b);

// Count since the previous call, then clear (POC getCount(true)).
int32_t encoder_take(void);

#ifdef __cplusplus
}
#endif
