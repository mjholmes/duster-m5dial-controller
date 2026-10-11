// EEZ native variables (components/ui/vars.h). Getters read the snapshot the
// ui task takes once per loop; setters are no-ops, because the UI may only
// change state through actions and control_request() (SPEC 4.2).

#include "ui_vars.h"
#include "vars.h"

#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "ui_vars";

static ui_model_t s_snap;
static char s_speed[4];
static char s_temp[8];
static char s_dtc_count[4];
static const char *s_comms_text = "";
static uint32_t s_speed_shown_ms;
static bool s_speed_shown_valid;
static bool s_started;

// Tyres / 4WD (docs/ui-tyres-4wd-spec.md 5, 6.1, 9).
static char s_tyre_text[TYRE_COUNT][6];   // "26" or "--"
static char s_slip_text[TYRE_COUNT][6];   // "+14%" / "-3%" / "0%" / "--"
static char s_wd_mode[6];                 // "AUTO" / "LOCK" / "2WD" / "--"
static char s_wd_torque[10];              // "120 Nm" / "-- Nm"
static tyre_severity_t s_tyre_sev[TYRE_COUNT];
static tyre_severity_t s_slip_sev[TYRE_COUNT];  // fed back each loop -- hysteresis state
static tyre_warn_state_t s_tyre_warn;
static bool s_tyre_warning_now;

void ui_vars_refresh(uint32_t now_ms)
{
    model_snapshot(&s_snap);

    // Speed is held between display updates to stop flicker (SPEC 5 note 5),
    // but a change of validity shows at once. The speed font has digits only,
    // so a stale speed is blank.
    bool valid = !s_snap.speed_stale;
    if (!s_started || valid != s_speed_shown_valid ||
        (uint32_t)(now_ms - s_speed_shown_ms) >= CONFIG_SPEED_DISPLAY_HOLD_MS) {
        if (valid) {
            snprintf(s_speed, sizeof(s_speed), "%u", s_snap.speed_mph);
        } else {
            s_speed[0] = '\0';
        }
        s_speed_shown_ms = now_ms;
        s_speed_shown_valid = valid;
    }

    if (s_snap.temp_stale) {
        snprintf(s_temp, sizeof(s_temp), "--");
    } else {
        snprintf(s_temp, sizeof(s_temp), "%d\xC2\xB0" "C", s_snap.ext_temp_c);
    }
    // DTC (SPEC 8.4). Until the OBD client exists (step 6) the status stays
    // DTC_NOT_READ and nothing is shown.
    if (s_snap.dtc.count > 0) {
        snprintf(s_dtc_count, sizeof(s_dtc_count), "%u", s_snap.dtc.count);
    } else {
        s_dtc_count[0] = '\0';
    }

    bool can_lost = s_snap.can_timeout || s_snap.can_bus_off;
    bool ra60_lost = s_snap.aux_timeout || s_snap.aux_fault;
    s_comms_text = can_lost && ra60_lost ? "BOTH" : can_lost ? "CAN" : ra60_lost ? "RA60" : "";

    // Tyres: per-wheel pressure severity and "psi" text (docs/ui-tyres-4wd-spec.md 5, 6.1).
    // Both under- and over-inflation warn -- see tyre_logic.h.
    static const tyre_psi_cfg_t psi_cfg = {
        .front = { .caution_under_psi = CONFIG_TYRE_CAUTION_PSI_FRONT, .alert_under_psi = CONFIG_TYRE_ALERT_PSI_FRONT,
                   .caution_over_psi  = CONFIG_TYRE_OVER_CAUTION_PSI_FRONT, .alert_over_psi = CONFIG_TYRE_OVER_ALERT_PSI_FRONT },
        .rear  = { .caution_under_psi = CONFIG_TYRE_CAUTION_PSI_REAR,  .alert_under_psi = CONFIG_TYRE_ALERT_PSI_REAR,
                   .caution_over_psi  = CONFIG_TYRE_OVER_CAUTION_PSI_REAR,  .alert_over_psi = CONFIG_TYRE_OVER_ALERT_PSI_REAR },
    };
    for (int pos = 0; pos < TYRE_COUNT; pos++) {
        bool front = (pos == TYRE_FL || pos == TYRE_FR);
        const tyre_psi_axle_cfg_t *axle = front ? &psi_cfg.front : &psi_cfg.rear;
        s_tyre_sev[pos] = tyre_pressure_severity(s_snap.tyre_pressures.raw[pos], s_snap.tyre_pressures.stale,
                                                  axle->caution_under_psi, axle->alert_under_psi,
                                                  axle->caution_over_psi, axle->alert_over_psi);
        if (s_tyre_sev[pos] == TYRE_SEV_NODATA) {
            snprintf(s_tyre_text[pos], sizeof(s_tyre_text[pos]), "--");
        } else {
            snprintf(s_tyre_text[pos], sizeof(s_tyre_text[pos]), "%d", tyre_psi(s_snap.tyre_pressures.raw[pos]));
        }
    }

    // Wheel slip: gated by speed, referenced to the median of all four.
    bool slip_gate = wheel_slip_gate_open(&s_snap.wheel_speeds, CONFIG_SLIP_SPEED_GATE_CENTI_KMH);
    uint16_t slip_ref = slip_gate ? wheel_speed_median(s_snap.wheel_speeds.raw) : 0;
    for (int pos = 0; pos < TYRE_COUNT; pos++) {
        if (!slip_gate) {
            s_slip_sev[pos] = TYRE_SEV_NODATA;
            snprintf(s_slip_text[pos], sizeof(s_slip_text[pos]), "--");
            continue;
        }
        int pct = wheel_slip_pct(s_snap.wheel_speeds.raw[pos], slip_ref);
        // wheel_slip_pct() already clamps to -99..99, but that's invisible
        // to the compiler across the function-call boundary; repeat the
        // clamp here so -Wformat-truncation can see the real bound on pct.
        if (pct > 99) {
            pct = 99;
        } else if (pct < -99) {
            pct = -99;
        }
        s_slip_sev[pos] = wheel_slip_severity(s_slip_sev[pos], false, pct < 0 ? -pct : pct,
                                               CONFIG_SLIP_CAUTION_PCT, CONFIG_SLIP_ALERT_PCT, CONFIG_SLIP_HYSTERESIS_PCT);
        if (pct > 0) {
            snprintf(s_slip_text[pos], sizeof(s_slip_text[pos]), "+%d%%", pct);
        } else {
            snprintf(s_slip_text[pos], sizeof(s_slip_text[pos]), "%d%%", pct);
        }
    }

    // 4WD header/footer (OQ-27: selector, not operating mode).
    if (s_snap.etc.stale || s_snap.etc.selector == 3) {
        snprintf(s_wd_mode, sizeof(s_wd_mode), "--");
    } else {
        static const char *names[3] = { "AUTO", "LOCK", "2WD" };
        snprintf(s_wd_mode, sizeof(s_wd_mode), "%s", names[s_snap.etc.selector]);
    }
    if (s_snap.etc.stale) {
        snprintf(s_wd_torque, sizeof(s_wd_torque), "-- Nm");
    } else {
        snprintf(s_wd_torque, sizeof(s_wd_torque), "%u Nm", s_snap.etc.torque_nm);
    }

    // Speedo tyre-warning icon (section 9), debounced. Reuses psi_cfg above.
    bool warn_condition = tyre_warning_condition(&s_snap.tyre_pressures, &psi_cfg);
    s_tyre_warning_now = tyre_warn_update(&s_tyre_warn, warn_condition, now_ms,
                                          CONFIG_TYRE_WARN_SET_MS, CONFIG_TYRE_WARN_CLEAR_MS);

    s_started = true;
}

const ui_model_t *ui_vars_model(void)
{
    return &s_snap;
}

tyre_severity_t ui_vars_tyre_pressure_severity(tyre_pos_t pos) { return s_tyre_sev[pos]; }
tyre_severity_t ui_vars_wheel_slip_severity(tyre_pos_t pos) { return s_slip_sev[pos]; }

const char *get_var_speed(void) { return s_speed; }
const char *get_var_temperature(void) { return s_temp; }
bool get_var_is_frost_risk(void) { return s_snap.frost_risk; }
bool get_var_is_below_freezing(void) { return s_snap.below_freezing; }
bool get_var_is_auto_mode(void) { return s_snap.mode == MODE_AUTO; }
// ACKed by the RA60: as best we know the light is on (SPEC 7.2).
bool get_var_opstatus_ledbar(void) { return s_snap.aux_actual.ledbar; }
bool get_var_opstatus_fog(void) { return s_snap.aux_actual.fog; }

bool get_var_is_dtc_present(void) { return s_snap.dtc.count > 0 || s_snap.dtc.mil_on; }
const char *get_var_dtc_count(void) { return s_dtc_count; }
bool get_var_is_comms_lost(void) { return s_comms_text[0] != '\0'; }
const char *get_var_comms_text(void) { return s_comms_text; }

const char *get_var_tyre_fl_text(void) { return s_tyre_text[TYRE_FL]; }
const char *get_var_tyre_fr_text(void) { return s_tyre_text[TYRE_FR]; }
const char *get_var_tyre_rl_text(void) { return s_tyre_text[TYRE_RL]; }
const char *get_var_tyre_rr_text(void) { return s_tyre_text[TYRE_RR]; }
const char *get_var_slip_fl_text(void) { return s_slip_text[TYRE_FL]; }
const char *get_var_slip_fr_text(void) { return s_slip_text[TYRE_FR]; }
const char *get_var_slip_rl_text(void) { return s_slip_text[TYRE_RL]; }
const char *get_var_slip_rr_text(void) { return s_slip_text[TYRE_RR]; }
const char *get_var_wd_mode_text(void) { return s_wd_mode; }
const char *get_var_wd_torque_text(void) { return s_wd_torque; }
bool get_var_is_tyre_warning(void) { return s_tyre_warning_now; }

static void ignored(const char *name)
{
    ESP_LOGD(TAG, "set_var_%s ignored (UI is read-only)", name);
}

void set_var_speed(const char *value) { (void)value; ignored("speed"); }
void set_var_temperature(const char *value) { (void)value; ignored("temperature"); }
void set_var_is_frost_risk(bool value) { (void)value; ignored("is_frost_risk"); }
void set_var_is_below_freezing(bool value) { (void)value; ignored("is_below_freezing"); }
void set_var_is_auto_mode(bool value) { (void)value; ignored("is_auto_mode"); }
void set_var_opstatus_ledbar(bool value) { (void)value; ignored("opstatus_ledbar"); }
void set_var_opstatus_fog(bool value) { (void)value; ignored("opstatus_fog"); }
void set_var_is_dtc_present(bool value) { (void)value; ignored("is_dtc_present"); }
void set_var_dtc_count(const char *value) { (void)value; ignored("dtc_count"); }
void set_var_is_comms_lost(bool value) { (void)value; ignored("is_comms_lost"); }
void set_var_comms_text(const char *value) { (void)value; ignored("comms_text"); }

void set_var_tyre_fl_text(const char *value) { (void)value; ignored("tyre_fl_text"); }
void set_var_tyre_fr_text(const char *value) { (void)value; ignored("tyre_fr_text"); }
void set_var_tyre_rl_text(const char *value) { (void)value; ignored("tyre_rl_text"); }
void set_var_tyre_rr_text(const char *value) { (void)value; ignored("tyre_rr_text"); }
void set_var_slip_fl_text(const char *value) { (void)value; ignored("slip_fl_text"); }
void set_var_slip_fr_text(const char *value) { (void)value; ignored("slip_fr_text"); }
void set_var_slip_rl_text(const char *value) { (void)value; ignored("slip_rl_text"); }
void set_var_slip_rr_text(const char *value) { (void)value; ignored("slip_rr_text"); }
void set_var_wd_mode_text(const char *value) { (void)value; ignored("wd_mode_text"); }
void set_var_wd_torque_text(const char *value) { (void)value; ignored("wd_torque_text"); }
void set_var_is_tyre_warning(bool value) { (void)value; ignored("is_tyre_warning"); }
