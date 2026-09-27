// Front button short/long press (SPEC 9.3, decided). Pure logic, host-tested.
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { BTN_EVT_NONE, BTN_EVT_SHORT, BTN_EVT_LONG } btn_evt_t;

typedef struct {
    bool     down;
    bool     long_fired;
    uint32_t down_ms;
} button_fsm_t;

// Feed the debounced button state each ui loop. SHORT fires on release before
// long_ms; LONG fires once when long_ms is reached while still held.
btn_evt_t button_fsm_step(button_fsm_t *b, bool pressed, uint32_t now_ms, uint32_t long_ms);
