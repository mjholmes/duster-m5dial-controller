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

    s_started = true;
}

const ui_model_t *ui_vars_model(void)
{
    return &s_snap;
}

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
