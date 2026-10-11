// Host tests for pure logic (no ESP-IDF). Build: see test/host/CMakeLists.txt.

#include <stdio.h>
#include "control_core.h"
#include "button_fsm.h"
#include "brightness.h"
#include "can_signals.h"
#include "tyre_logic.h"

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

static const ctrl_cfg_t k_cfg = {
    .link_timeout_ms = { [LINK_CAN_LIGHTS] = 750, [LINK_AUX] = 1000 },
    .aux_mismatch_ms = 1000,
};

// Core after power-on with every link delivering data at t = 1000 ms.
static ctrl_core_t make_core(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c, &k_cfg);
    const uint32_t fresh[LINK_COUNT] = { 1000, 1000 };
    ctrl_core_tick(&c, fresh, 1000);
    return c;
}

static void set_auto(ctrl_core_t *c)
{
    ctrl_req_t m = { .type = REQ_SET_MODE, .mode = MODE_AUTO };
    ctrl_core_apply(c, &m);
}

static void test_power_on(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c, &k_cfg);
    CHECK(c.link_down[LINK_CAN_LIGHTS] && c.link_down[LINK_AUX]);  // down until data arrives
    CHECK(c.mode == MODE_MANUAL);
    CHECK(lights(c.manual_state, false, false));
    CHECK(lights(c.aux_commanded, false, false));
}

static void test_toggle_in_manual(void)
{
    ctrl_core_t c = make_core();
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
    ctrl_core_t c = make_core();
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
    ctrl_core_t c = make_core();
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
    ctrl_core_t c = make_core();
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
    ctrl_core_t c = make_core();
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

static void test_can_timeout_in_auto_forces_manual_off(void)
{
    ctrl_core_t c = make_core();
    set_auto(&c);
    CHECK(c.mode == MODE_AUTO);
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ .main_beam = true, .front_fog = true });
    CHECK(lights(c.aux_commanded, true, true));

    uint32_t rx[LINK_COUNT] = { 1000, 1700 };
    CHECK(!ctrl_core_tick(&c, rx, 1749));             // 749 ms: still up
    CHECK(!c.link_down[LINK_CAN_LIGHTS] && c.mode == MODE_AUTO);
    CHECK(ctrl_core_tick(&c, rx, 1750));              // 750 ms: lost
    CHECK(c.link_down[LINK_CAN_LIGHTS]);
    CHECK(c.mode == MODE_MANUAL && lights(c.manual_state, false, false));
    CHECK(ctrl_core_update_commanded(&c, (vehicle_lights_t){ .main_beam = true, .front_fog = true }));
    CHECK(lights(c.aux_commanded, false, false));     // stale inputs no longer drive lights
}

static void test_can_timeout_in_manual_is_warning_only(void)
{
    ctrl_core_t c = make_core();
    ctrl_req_t t = req(REQ_TOGGLE_LIGHT);
    t.light = AUX_LIGHT_FOG;
    ctrl_core_apply(&c, &t);
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 });

    const uint32_t rx[LINK_COUNT] = { 1000, 2500 };
    CHECK(!ctrl_core_tick(&c, rx, 2500));
    CHECK(c.link_down[LINK_CAN_LIGHTS]);
    CHECK(c.mode == MODE_MANUAL && lights(c.manual_state, false, true));  // fog stays on
}

static void test_auto_refused_while_can_down_then_allowed(void)
{
    ctrl_core_t c;
    ctrl_core_init(&c, &k_cfg);                        // no CAN data yet
    ctrl_req_t m = req(REQ_SET_MODE);
    m.mode = MODE_AUTO;
    m.seq = 5;
    CHECK(!ctrl_core_apply(&c, &m));
    CHECK(c.mode == MODE_MANUAL && c.last_applied_req_seq == 0);

    const uint32_t rx[LINK_COUNT] = { 3000, 3000 };
    ctrl_core_tick(&c, rx, 3000);                      // CAN arrives
    CHECK(ctrl_core_apply(&c, &m) && c.mode == MODE_AUTO);
}

static void test_can_recovery_stays_manual(void)
{
    ctrl_core_t c = make_core();
    set_auto(&c);
    uint32_t rx[LINK_COUNT] = { 1000, 2000 };
    ctrl_core_tick(&c, rx, 2000);                      // lost, forced MANUAL
    rx[LINK_CAN_LIGHTS] = 2100;
    CHECK(!ctrl_core_tick(&c, rx, 2100));              // back
    CHECK(!c.link_down[LINK_CAN_LIGHTS]);
    CHECK(c.mode == MODE_MANUAL);                      // no automatic return to AUTO
}

static void test_aux_timeout_keeps_mode(void)
{
    ctrl_core_t c = make_core();
    set_auto(&c);
    const uint32_t rx[LINK_COUNT] = { 2100, 1000 };   // CAN fresh, no RA60 ACK for 1100 ms
    CHECK(!ctrl_core_tick(&c, rx, 2100));
    CHECK(c.link_down[LINK_AUX] && !c.link_down[LINK_CAN_LIGHTS]);
    CHECK(c.mode == MODE_AUTO);
}

static void test_aux_mismatch(void)
{
    ctrl_core_t c = make_core();
    ctrl_req_t t = req(REQ_TOGGLE_LIGHT);
    ctrl_core_apply(&c, &t);
    ctrl_core_update_commanded(&c, (vehicle_lights_t){ 0 });   // LED bar commanded on, not ACKed

    uint32_t rx[LINK_COUNT] = { 5000, 5000 };
    ctrl_core_tick(&c, rx, 5000);
    CHECK(!c.aux_mismatch);                            // grace period starts
    rx[0] = rx[1] = 5999;
    ctrl_core_tick(&c, rx, 5999);
    CHECK(!c.aux_mismatch);
    rx[0] = rx[1] = 6000;
    ctrl_core_tick(&c, rx, 6000);
    CHECK(c.aux_mismatch);                             // 1000 ms without ACK
    ctrl_core_aux_acked(&c, c.aux_commanded);
    ctrl_core_tick(&c, rx, 6010);
    CHECK(!c.aux_mismatch);
}

static void test_link_timeout_across_ms_wrap(void)
{
    ctrl_core_t c = make_core();
    set_auto(&c);
    const uint32_t rx[LINK_COUNT] = { 0xFFFFFF00u, 0xFFFFFF00u };
    ctrl_core_tick(&c, rx, 0x00000100u);               // 512 ms later, across the wrap
    CHECK(!c.link_down[LINK_CAN_LIGHTS] && c.mode == MODE_AUTO);
    ctrl_core_tick(&c, rx, 0x00000300u);               // 1024 ms later
    CHECK(c.link_down[LINK_CAN_LIGHTS] && c.mode == MODE_MANUAL);
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

    // Day/night from the headlight switch. Sequences seen on the car.
    bool n = false;
    n = brightness_night_step(n, true, false, false); CHECK(!n);  // lights off
    n = brightness_night_step(n, true, false, true);  CHECK(!n);  // daytime flash stays day
    n = brightness_night_step(n, true, false, false); CHECK(!n);
    n = brightness_night_step(n, true, true, false);  CHECK(n);   // dipped on: night
    n = brightness_night_step(n, true, false, true);  CHECK(n);   // main on clears dipped bit: stays night
    n = brightness_night_step(n, true, true, false);  CHECK(n);   // back to dipped
    n = brightness_night_step(n, false, false, false); CHECK(n);  // CAN lost: hold
    n = brightness_night_step(n, true, false, false); CHECK(!n);  // lights off: day
}

static void test_can_decode(void)
{
    can_decoded_t d;

    // Field notes: 0x354 27 10 = 100.00 km/h = 62.1 mph.
    const uint8_t speed[] = { 0x27, 0x10 };
    CHECK(can_decode(0x354, false, false, speed, 2, &d) == CAN_DEC_OK);
    CHECK(d.sig == CAN_SIG_SPEED && d.speed.raw_centi_kmh == 10000 && d.speed.mph == 62);
    const uint8_t fast[] = { 0xFF, 0xFF };   // 655 km/h clamps to 99 mph
    CHECK(can_decode(0x354, false, false, fast, 2, &d) == CAN_DEC_OK && d.speed.mph == 99);
    const uint8_t stopped[] = { 0x00, 0x00 };
    CHECK(can_decode(0x354, false, false, stopped, 2, &d) == CAN_DEC_OK && d.speed.mph == 0);
    CHECK(can_decode(0x354, false, false, speed, 1, &d) == CAN_DEC_MALFORMED && d.sig == CAN_SIG_SPEED);

    // Temperature: byte 0 - 40.
    // Captured on the car: 0x3B7 31 FA FF CF FE 40 = 9 degC (dash showed 9).
    const uint8_t temp[6] = { 0x31, 0xFA, 0xFF, 0xCF, 0xFE, 0x40 };
    CHECK(can_decode(0x3B7, false, false, temp, 6, &d) == CAN_DEC_OK && d.ext_temp_c == 9);
    const uint8_t cold[6] = { 0x1E };        // -10 degC
    CHECK(can_decode(0x3B7, false, false, cold, 6, &d) == CAN_DEC_OK && d.ext_temp_c == -10);
    CHECK(can_decode(0x3B7, false, false, temp, 5, &d) == CAN_DEC_MALFORMED);
    CHECK(can_decode(0x3BF, false, false, temp, 6, &d) == CAN_DEC_NOT_SUBSCRIBED);

    // Field notes: 0x5DE byte 0 0x09 = main beam and front fog on.
    const uint8_t lights[8] = { 0x09 };
    CHECK(can_decode(0x5DE, false, false, lights, 8, &d) == CAN_DEC_OK);
    CHECK(d.lights.main_beam && d.lights.front_fog && !d.lights.dipped_beam);
    const uint8_t dipped[8] = { 0x02 };
    CHECK(can_decode(0x5DE, false, false, dipped, 8, &d) == CAN_DEC_OK);
    CHECK(!d.lights.main_beam && !d.lights.front_fog && d.lights.dipped_beam);

    // Ignition enum: known values OK, others counted as unknown.
    const uint8_t running[8] = { 0xC7 };
    CHECK(can_decode(0x350, false, false, running, 8, &d) == CAN_DEC_OK && d.ignition_raw == 0xC7);
    const uint8_t odd[8] = { 0xC2 };
    CHECK(can_decode(0x350, false, false, odd, 8, &d) == CAN_DEC_UNKNOWN_VALUE && d.sig == CAN_SIG_IGNITION);

    // Not subscribed, extended IDs, RTR.
    CHECK(can_decode(0x186, false, false, lights, 8, &d) == CAN_DEC_NOT_SUBSCRIBED);
    CHECK(can_decode(0x5DE, true, false, lights, 8, &d) == CAN_DEC_NOT_SUBSCRIBED);
    CHECK(can_decode(0x5DE, false, true, lights, 8, &d) == CAN_DEC_MALFORMED);
    CHECK(can_is_obd_response(0x7E8) && !can_is_obd_response(0x7E0));
}

// Vectors from docs/ui-tyres-4wd-spec.md section 3.5 (real frames, drive-log.can).
static void test_tyre_decode(void)
{
    can_decoded_t d;

    const uint8_t tpms[7] = { 0x00, 0x00, 0x3C, 0x52, 0x3E, 0x36, 0x00 };
    CHECK(can_decode(0x673, false, false, tpms, 7, &d) == CAN_DEC_OK);
    CHECK(!d.tpms.failure);
    CHECK(d.tpms.pressure_raw[TYRE_RR] == 0x3C && d.tpms.pressure_raw[TYRE_RL] == 0x52 &&
          d.tpms.pressure_raw[TYRE_FR] == 0x3E && d.tpms.pressure_raw[TYRE_FL] == 0x36);
    CHECK((int)d.tpms.pressure_raw[TYRE_RR] * 30 == 1800 && (int)d.tpms.pressure_raw[TYRE_RL] * 30 == 2460 &&
          (int)d.tpms.pressure_raw[TYRE_FR] * 30 == 1860 && (int)d.tpms.pressure_raw[TYRE_FL] * 30 == 1620);

    const uint8_t tpms_novalue[7] = { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
    CHECK(can_decode(0x673, false, false, tpms_novalue, 7, &d) == CAN_DEC_OK);
    CHECK(d.tpms.pressure_raw[TYRE_FL] == 0xFF && d.tpms.pressure_raw[TYRE_FR] == 0xFF &&
          d.tpms.pressure_raw[TYRE_RL] == 0xFF && d.tpms.pressure_raw[TYRE_RR] == 0xFF);
    CHECK(can_decode(0x673, false, false, tpms, 6, &d) == CAN_DEC_MALFORMED);

    const uint8_t wheel_front[8] = { 0x0B, 0x29, 0x0B, 0x40, 0x05, 0xE1, 0x02, 0x98 };
    CHECK(can_decode(0x29A, false, false, wheel_front, 8, &d) == CAN_DEC_OK);
    CHECK(d.wheel_speed_front.fr_raw == 0x0B29 && d.wheel_speed_front.fl_raw == 0x0B40 &&
          d.wheel_speed_front.speed_raw_centi_kmh == 1505);

    const uint8_t wheel_rear[8] = { 0x0B, 0x56, 0x0B, 0x40, 0xFF, 0xFF, 0xFF, 0xFF };
    CHECK(can_decode(0x29C, false, false, wheel_rear, 8, &d) == CAN_DEC_OK);
    CHECK(d.wheel_speed_rear.rl_raw == 0x0B56 && d.wheel_speed_rear.rr_raw == 0x0B40);

    const uint8_t etc_open_auto[4] = { 0x47, 0xFF, 0x80, 0x80 };
    CHECK(can_decode(0x1AA, false, false, etc_open_auto, 4, &d) == CAN_DEC_OK);
    CHECK(d.etc.torque_nm == 10 && d.etc.coupler == 0 && d.etc.mode == 0 && d.etc.selector == 0 && !d.etc.failure);

    const uint8_t etc_2wd_startup[4] = { 0x07, 0xFF, 0x80, 0xAB };
    CHECK(can_decode(0x1AA, false, false, etc_2wd_startup, 4, &d) == CAN_DEC_OK);
    CHECK(d.etc.coupler == 3 && d.etc.mode == 2 && d.etc.selector == 2);
    CHECK(can_decode(0x1AA, false, false, etc_open_auto, 3, &d) == CAN_DEC_MALFORMED);
}

static void test_tyre_logic(void)
{
    // psi vector, docs/ui-tyres-4wd-spec.md 3.5: 0x3C->26, 0x52->36, 0x3E->27, 0x36->23.
    CHECK(tyre_psi(0x3C) == 26 && tyre_psi(0x52) == 36 && tyre_psi(0x3E) == 27 && tyre_psi(0x36) == 23);

    // Pressure severity: default thresholds, both axles caution 27 / alert 23.
    CHECK(tyre_pressure_severity(0xFF, false, 27, 23) == TYRE_SEV_NODATA);  // no value
    CHECK(tyre_pressure_severity(0x3C, true, 27, 23) == TYRE_SEV_NODATA);   // stale
    CHECK(tyre_pressure_severity(0x32, false, 27, 23) == TYRE_SEV_ALERT);   // 22 psi, below alert
    CHECK(tyre_pressure_severity(0x36, false, 27, 23) == TYRE_SEV_CAUTION); // 23 psi: not < alert, < caution
    CHECK(tyre_pressure_severity(0x3C, false, 27, 23) == TYRE_SEV_CAUTION); // 26 psi
    CHECK(tyre_pressure_severity(0x3E, false, 27, 23) == TYRE_SEV_OK);      // 27 psi: not < caution

    // tyre_warning_condition: the spec's own 0x673 vector (RR 26, RL 36, FR 27, FL 23 psi)
    // with default thresholds -> RR and FL are CAUTION, so it must raise.
    tyre_pressures_t tp = { .raw = { [TYRE_FL] = 0x36, [TYRE_FR] = 0x3E, [TYRE_RL] = 0x52, [TYRE_RR] = 0x3C } };
    tyre_psi_cfg_t psi_cfg = { .caution_psi_front = 27, .alert_psi_front = 23, .caution_psi_rear = 27, .alert_psi_rear = 23 };
    CHECK(tyre_warning_condition(&tp, &psi_cfg));

    tyre_pressures_t tp_stale = tp;
    tp_stale.stale = true;
    CHECK(!tyre_warning_condition(&tp_stale, &psi_cfg));  // stale never raises

    tyre_pressures_t tp_ok = { .raw = { [TYRE_FL] = 0x3E, [TYRE_FR] = 0x3E, [TYRE_RL] = 0x3E, [TYRE_RR] = 0x3E } };
    CHECK(!tyre_warning_condition(&tp_ok, &psi_cfg));  // all 27 psi: OK, no flat/failure

    tyre_pressures_t tp_flat = tp_ok;
    tp_flat.state[TYRE_RL] = 5;  // flat, even though pressure itself reads OK
    CHECK(tyre_warning_condition(&tp_flat, &psi_cfg));

    tyre_pressures_t tp_failure = tp_ok;
    tp_failure.failure = true;
    CHECK(tyre_warning_condition(&tp_failure, &psi_cfg));

    // Wheel speed median and slip, spec vector: FL 2880, FR 2857, RL 2902, RR 2880 raw.
    uint16_t speeds[TYRE_COUNT] = { [TYRE_FL] = 2880, [TYRE_FR] = 2857, [TYRE_RL] = 2902, [TYRE_RR] = 2880 };
    uint16_t median = wheel_speed_median(speeds);
    CHECK(median == 2880);
    CHECK(wheel_slip_pct(1100, 1000) == 10);
    CHECK(wheel_slip_pct(900, 1000) == -10);
    CHECK(wheel_slip_pct(3000, 1000) == 99);   // clamp high
    CHECK(wheel_slip_pct(0, 1000) == -99);     // clamp low
    CHECK(wheel_slip_pct(123, 0) == 0);        // guard against /0

    wheel_speeds_t ws = { .speed_centi_kmh = 305, .stale = false };
    CHECK(wheel_slip_gate_open(&ws, 300));
    ws.speed_centi_kmh = 299;
    CHECK(!wheel_slip_gate_open(&ws, 300));
    ws.speed_centi_kmh = 400;
    ws.stale = true;
    CHECK(!wheel_slip_gate_open(&ws, 300));

    // Slip severity hysteresis: caution 5%, alert 10%, hysteresis 2%.
    CHECK(wheel_slip_severity(TYRE_SEV_OK, true, 0, 5, 10, 2) == TYRE_SEV_NODATA);
    CHECK(wheel_slip_severity(TYRE_SEV_OK, false, 4, 5, 10, 2) == TYRE_SEV_OK);
    CHECK(wheel_slip_severity(TYRE_SEV_OK, false, 5, 5, 10, 2) == TYRE_SEV_CAUTION);     // rising: plain threshold
    CHECK(wheel_slip_severity(TYRE_SEV_CAUTION, false, 4, 5, 10, 2) == TYRE_SEV_CAUTION); // falling: held by hysteresis
    CHECK(wheel_slip_severity(TYRE_SEV_CAUTION, false, 2, 5, 10, 2) == TYRE_SEV_OK);      // below caution - hysteresis
    CHECK(wheel_slip_severity(TYRE_SEV_CAUTION, false, 10, 5, 10, 2) == TYRE_SEV_ALERT);  // rising through to alert
    CHECK(wheel_slip_severity(TYRE_SEV_ALERT, false, 9, 5, 10, 2) == TYRE_SEV_ALERT);     // falling: held by hysteresis
    CHECK(wheel_slip_severity(TYRE_SEV_ALERT, false, 7, 5, 10, 2) == TYRE_SEV_CAUTION);   // drops to caution band
    CHECK(wheel_slip_severity(TYRE_SEV_ALERT, false, 2, 5, 10, 2) == TYRE_SEV_OK);        // drops straight to OK

    // Tyre-warning debounce: set 2000ms, clear 10000ms.
    tyre_warn_state_t st = {0};
    CHECK(!tyre_warn_update(&st, true, 0, 2000, 10000));
    CHECK(!tyre_warn_update(&st, true, 1999, 2000, 10000));
    CHECK(tyre_warn_update(&st, true, 2000, 2000, 10000));
    CHECK(tyre_warn_update(&st, false, 2500, 2000, 10000));       // still raised, clear timer just started
    CHECK(tyre_warn_update(&st, false, 2500 + 9999, 2000, 10000));
    CHECK(!tyre_warn_update(&st, false, 2500 + 10000, 2000, 10000));

    tyre_warn_state_t flicker = {0};
    CHECK(!tyre_warn_update(&flicker, true, 0, 2000, 10000));
    CHECK(!tyre_warn_update(&flicker, true, 1000, 2000, 10000));  // held 1000ms, not yet 2000
    CHECK(!tyre_warn_update(&flicker, false, 1500, 2000, 10000)); // flips back: timer resets
    CHECK(!tyre_warn_update(&flicker, false, 3400, 2000, 10000)); // never raised, so nothing to clear
}

void run_obd_tests(void);
extern int g_obd_checks, g_obd_failures;

int main(void)
{
    test_power_on();
    test_toggle_in_manual();
    test_light_requests_rejected_in_auto();
    test_auto_follows_vehicle();
    test_auto_to_manual_bumpless();
    test_manual_all_off();
    test_can_timeout_in_auto_forces_manual_off();
    test_can_timeout_in_manual_is_warning_only();
    test_auto_refused_while_can_down_then_allowed();
    test_can_recovery_stays_manual();
    test_aux_timeout_keeps_mode();
    test_aux_mismatch();
    test_link_timeout_across_ms_wrap();
    test_temp_flag_hysteresis();
    test_button();
    test_brightness();
    test_can_decode();
    test_tyre_decode();
    test_tyre_logic();
    run_obd_tests();
    s_checks += g_obd_checks;
    s_failures += g_obd_failures;
    printf("%d/%d checks passed\n", s_checks - s_failures, s_checks);
    return s_failures ? 1 : 0;
}
