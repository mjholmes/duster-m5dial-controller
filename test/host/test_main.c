// Host tests for pure logic (no ESP-IDF). Build: see test/host/CMakeLists.txt.

#include <stdio.h>
#include "control_core.h"
#include "button_fsm.h"
#include "brightness.h"

static int s_checks, s_failures;

#define CHECK(cond) do { \
    s_checks++; \
    if (!(cond)) { s_failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

static ctrl_req_t req(req_type_t type)
{
    return (ctrl_req_t){ .type = type, .src = REQ_SRC_LOCAL_UI };
}

static bool lights(aux_lights_t l, bool ledbar, bool fog)
{
    return l.ledbar == ledbar && l.fog == fog;
}

static void test_power_on(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    CHECK(c.mode == MODE_MANUAL);
    CHECK(lights(c.manual_state, false, false));
    CHECK(lights(c.aux_commanded, false, false));
}

static void test_toggle_in_manual(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    ctrl_req_t r = req(REQ_TOGGLE_LIGHT);
    r.light = AUX_LIGHT_FOG;
    r.seq = 7;
    CHECK(ctrl_core_apply(&c, &r));
    CHECK(c.last_applied_req_seq == 7);
    CHECK(ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 }));
    CHECK(lights(c.aux_commanded, false, true));
    CHECK(!ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 }));  // no change, no repost
    CHECK(ctrl_core_apply(&c, &r));
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 });
    CHECK(lights(c.aux_commanded, false, false));
}

static void test_light_requests_rejected_in_auto(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    ctrl_req_t m = req(REQ_SET_MODE);
    m.mode = MODE_AUTO;
    m.seq = 1;
    CHECK(ctrl_core_apply(&c, &m));
    ctrl_req_t t = req(REQ_TOGGLE_LIGHT);
    t.seq = 2;
    CHECK(!ctrl_core_apply(&c, &t));
    ctrl_req_t s = req(REQ_SET_LIGHT);
    s.on = true;
    CHECK(!ctrl_core_apply(&c, &s));
    CHECK(c.last_applied_req_seq == 1);
    CHECK(lights(c.manual_state, false, false));
}

static void test_auto_follows_vehicle(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    ctrl_req_t m = req(REQ_SET_MODE);
    m.mode = MODE_AUTO;
    ctrl_core_apply(&c, &m);
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ .main_beam = true });
    CHECK(lights(c.aux_commanded, true, false));
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ .front_fog = true });
    CHECK(lights(c.aux_commanded, false, true));
}

static void test_auto_to_manual_bumpless(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    ctrl_req_t m = req(REQ_SET_MODE);
    m.mode = MODE_AUTO;
    ctrl_core_apply(&c, &m);
    vehicle_lights_t veh = { .main_beam = true, .front_fog = true };
    ctrl_core_update_commanded(&c, veh);
    m.mode = MODE_MANUAL;
    ctrl_core_apply(&c, &m);
    CHECK(lights(c.manual_state, true, true));
    CHECK(!ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 }));  // lights unchanged at switch
    CHECK(lights(c.aux_commanded, true, true));
}

static void test_manual_all_off(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c);
    ctrl_req_t m = req(REQ_SET_MODE);
    m.mode = MODE_AUTO;
    ctrl_core_apply(&c, &m);
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ .main_beam = true, .front_fog = true });
    ctrl_req_t off = req(REQ_MANUAL_ALL_OFF);
    CHECK(ctrl_core_apply(&c, &off));
    CHECK(c.mode == MODE_MANUAL);
    CHECK(lights(c.manual_state, false, false));
    CHECK(ctrl_core_update_commanded(&c, (vehicle_lights_t){ .main_beam = true, .front_fog = true }));
    CHECK(lights(c.aux_commanded, false, false));
}

static void test_temp_flag_hysteresis(void)
{
    // Frost risk: on <= 4, off >= 6.
    bool f = false;
    f = ctrl_temp_flag(f, false, 7, 4, 2); CHECK(!f);
    f = ctrl_temp_flag(f, false, 5, 4, 2); CHECK(!f);
    f = ctrl_temp_flag(f, false, 4, 4, 2); CHECK(f);
    f = ctrl_temp_flag(f, false, 5, 4, 2); CHECK(f);
    f = ctrl_temp_flag(f, false, 6, 4, 2); CHECK(!f);
    f = ctrl_temp_flag(f, false, -10, 4, 2); CHECK(f);
    f = ctrl_temp_flag(f, true, -10, 4, 2); CHECK(!f);  // stale
    // Below freezing: on <= 0, off >= 2.
    CHECK(ctrl_temp_flag(false, false, 0, 0, 2));
    CHECK(ctrl_temp_flag(true, false, 1, 0, 2));
    CHECK(!ctrl_temp_flag(true, false, 2, 0, 2));
}

static void test_button(void)
{
    button_fsm_t b = { 0 };
    CHECK(button_fsm_step(&b, false, 0, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, true, 10, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, true, 500, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, false, 600, 1000) == BTN_EVT_SHORT);

    CHECK(button_fsm_step(&b, true, 2000, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, true, 2999, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, true, 3000, 1000) == BTN_EVT_LONG);
    CHECK(button_fsm_step(&b, true, 5000, 1000) == BTN_EVT_NONE);  // fires once
    CHECK(button_fsm_step(&b, false, 5010, 1000) == BTN_EVT_NONE); // no short after long

    // Millisecond counter wrap.
    CHECK(button_fsm_step(&b, true, 0xFFFFFF00u, 1000) == BTN_EVT_NONE);
    CHECK(button_fsm_step(&b, true, 0x00000300u, 1000) == BTN_EVT_LONG);
}

static void test_brightness(void)
{
    const brightness_cfg_t cfg = { .min_pct = 10, .max_pct = 100, .step_pct = 5 };
    CHECK(brightness_step(80, 1, &cfg) == 85);
    CHECK(brightness_step(80, -2, &cfg) == 70);
    CHECK(brightness_step(95, 3, &cfg) == 100);      // clamp high
    CHECK(brightness_step(15, -5, &cfg) == 10);      // clamp low, never off
    CHECK(brightness_step(50, 0, &cfg) == 50);
    CHECK(brightness_step(50, -100000, &cfg) == 10); // large delta, no overflow
    CHECK(brightness_to_level(100) == 255);
    CHECK(brightness_to_level(0) == 0);
    CHECK(brightness_to_level(10) == 26);

    // Night dim: points off the setting, floored at the minimum.
    CHECK(brightness_effective(80, false, 20, 10) == 80);
    CHECK(brightness_effective(80, true, 20, 10) == 60);
    CHECK(brightness_effective(25, true, 20, 10) == 10);
    CHECK(brightness_effective(80, true, 0, 10) == 80);
}

int main(void)
{
    test_power_on();
    test_toggle_in_manual();
    test_light_requests_rejected_in_auto();
    test_auto_follows_vehicle();
    test_auto_to_manual_bumpless();
    test_manual_all_off();
    test_temp_flag_hysteresis();
    test_button();
    test_brightness();
    printf("%d/%d checks passed\n", s_checks - s_failures, s_checks);
    return s_failures ? 1 : 0;
}
