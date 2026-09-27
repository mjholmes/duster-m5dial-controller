#include "button_fsm.h"

btn_evt_t button_fsm_step(button_fsm_t *b, bool pressed, uint32_t now_ms, uint32_t long_ms)
{
    if (pressed && !b->down) {
        b->down = true;
        b->long_fired = false;
        b->down_ms = now_ms;
        return BTN_EVT_NONE;
    }
    if (pressed) {
        if (!b->long_fired && (uint32_t)(now_ms - b->down_ms) >= long_ms) {
            b->long_fired = true;
            return BTN_EVT_LONG;
        }
        return BTN_EVT_NONE;
    }
    if (b->down) {
        b->down = false;
        return b->long_fired ? BTN_EVT_NONE : BTN_EVT_SHORT;
    }
    return BTN_EVT_NONE;
}
