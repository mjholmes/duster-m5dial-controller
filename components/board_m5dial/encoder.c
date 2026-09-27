// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright 2024 mzyy94
//
// Quadrature encoder on PCNT. Ported to C from M5Dial-LVGL v0.0.1
// (https://github.com/mzyy94/M5Dial-LVGL, src/encoder.cpp, MIT licence) as
// used and hardware-tested in ../m5dial-espnow-test. Decoding logic is
// unchanged: same unit limits, 1000 ns glitch filter, channel edge/level
// actions and pin roles. Only the interface changed (C functions instead of
// a class). No change was needed for ESP-IDF 5.5.5.
//
// MIT License
//
// Copyright (c) 2024 mzyy94
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "encoder.h"
#include "driver/pulse_cnt.h"
#include "esp_check.h"

#define PCNT_LOW_LIMIT  -32768
#define PCNT_HIGH_LIMIT 32767

static pcnt_unit_handle_t s_unit;

esp_err_t encoder_init(gpio_num_t pin_a, gpio_num_t pin_b)
{
    pcnt_unit_config_t unit_config = {
        .low_limit = PCNT_LOW_LIMIT,
        .high_limit = PCNT_HIGH_LIMIT,
        .intr_priority = 1,
        .flags = {
            .accum_count = 1,
        },
    };
    ESP_RETURN_ON_ERROR(pcnt_new_unit(&unit_config, &s_unit), "enc", "unit");

    const pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_RETURN_ON_ERROR(pcnt_unit_set_glitch_filter(s_unit, &filter_config), "enc", "filter");

    const pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = pin_a,
        .level_gpio_num = pin_b,
    };
    pcnt_channel_handle_t chan_a;
    ESP_RETURN_ON_ERROR(pcnt_new_channel(s_unit, &chan_a_config, &chan_a), "enc", "chan a");

    const pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = pin_b,
        .level_gpio_num = pin_a,
    };
    pcnt_channel_handle_t chan_b;
    ESP_RETURN_ON_ERROR(pcnt_new_channel(s_unit, &chan_b_config, &chan_b), "enc", "chan b");

    ESP_RETURN_ON_ERROR(pcnt_channel_set_edge_action(chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD), "enc", "a edge");
    ESP_RETURN_ON_ERROR(pcnt_channel_set_level_action(chan_a, PCNT_CHANNEL_LEVEL_ACTION_HOLD, PCNT_CHANNEL_LEVEL_ACTION_INVERSE), "enc", "a level");
    ESP_RETURN_ON_ERROR(pcnt_channel_set_edge_action(chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD), "enc", "b edge");
    ESP_RETURN_ON_ERROR(pcnt_channel_set_level_action(chan_b, PCNT_CHANNEL_LEVEL_ACTION_HOLD, PCNT_CHANNEL_LEVEL_ACTION_INVERSE), "enc", "b level");

    ESP_RETURN_ON_ERROR(pcnt_unit_enable(s_unit), "enc", "enable");
    ESP_RETURN_ON_ERROR(pcnt_unit_clear_count(s_unit), "enc", "clear");
    return pcnt_unit_start(s_unit);
}

int32_t encoder_take(void)
{
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(s_unit, &count));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_unit));
    return count;
}
