#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_SPEED = 0,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE = 1,
    FLOW_GLOBAL_VARIABLE_IS_FROST_RISK = 2,
    FLOW_GLOBAL_VARIABLE_IS_BELOW_FREEZING = 3,
    FLOW_GLOBAL_VARIABLE_IS_AUTO_MODE = 4,
    FLOW_GLOBAL_VARIABLE_OPSTATUS_FOG = 5,
    FLOW_GLOBAL_VARIABLE_OPSTATUS_LEDBAR = 6,
    FLOW_GLOBAL_VARIABLE_IS_DTC_PRESENT = 7,
    FLOW_GLOBAL_VARIABLE_DTC_COUNT = 8,
    FLOW_GLOBAL_VARIABLE_IS_COMMS_LOST = 9,
    FLOW_GLOBAL_VARIABLE_COMMS_TEXT = 10,
    FLOW_GLOBAL_VARIABLE_TYRE_FL_TEXT = 11,
    FLOW_GLOBAL_VARIABLE_SLIP_FL_TEXT = 12,
    FLOW_GLOBAL_VARIABLE_TYRE_FR_TEXT = 13,
    FLOW_GLOBAL_VARIABLE_SLIP_FR_TEXT = 14,
    FLOW_GLOBAL_VARIABLE_TYRE_RL_TEXT = 15,
    FLOW_GLOBAL_VARIABLE_SLIP_RL_TEXT = 16,
    FLOW_GLOBAL_VARIABLE_TYRE_RR_TEXT = 17,
    FLOW_GLOBAL_VARIABLE_SLIP_RR_TEXT = 18,
    FLOW_GLOBAL_VARIABLE_WD_MODE_TEXT = 19,
    FLOW_GLOBAL_VARIABLE_WD_TORQUE_TEXT = 20,
    FLOW_GLOBAL_VARIABLE_IS_TYRE_WARNING = 21,
    FLOW_GLOBAL_VARIABLE_OTA_VERSION_TEXT = 22,
    FLOW_GLOBAL_VARIABLE_OTA_STATUS_TEXT = 23,
    FLOW_GLOBAL_VARIABLE_OTA_ACTION_TEXT = 24,
    FLOW_GLOBAL_VARIABLE_IS_OTA_BETA_CHANNEL = 25,
    FLOW_GLOBAL_VARIABLE_IS_OTA_STABLE_CHANNEL = 26,
    FLOW_GLOBAL_VARIABLE_IS_OTA_BUSY = 27,
    FLOW_GLOBAL_VARIABLE_IS_OTA_ERROR = 28
};

// Native global variables

extern const char *get_var_speed();
extern void set_var_speed(const char *value);
extern const char *get_var_temperature();
extern void set_var_temperature(const char *value);
extern bool get_var_is_frost_risk();
extern void set_var_is_frost_risk(bool value);
extern bool get_var_is_below_freezing();
extern void set_var_is_below_freezing(bool value);
extern bool get_var_is_auto_mode();
extern void set_var_is_auto_mode(bool value);
extern bool get_var_opstatus_fog();
extern void set_var_opstatus_fog(bool value);
extern bool get_var_opstatus_ledbar();
extern void set_var_opstatus_ledbar(bool value);
extern bool get_var_is_dtc_present();
extern void set_var_is_dtc_present(bool value);
extern const char *get_var_dtc_count();
extern void set_var_dtc_count(const char *value);
extern bool get_var_is_comms_lost();
extern void set_var_is_comms_lost(bool value);
extern const char *get_var_comms_text();
extern void set_var_comms_text(const char *value);
extern const char *get_var_tyre_fl_text();
extern void set_var_tyre_fl_text(const char *value);
extern const char *get_var_slip_fl_text();
extern void set_var_slip_fl_text(const char *value);
extern const char *get_var_tyre_fr_text();
extern void set_var_tyre_fr_text(const char *value);
extern const char *get_var_slip_fr_text();
extern void set_var_slip_fr_text(const char *value);
extern const char *get_var_tyre_rl_text();
extern void set_var_tyre_rl_text(const char *value);
extern const char *get_var_slip_rl_text();
extern void set_var_slip_rl_text(const char *value);
extern const char *get_var_tyre_rr_text();
extern void set_var_tyre_rr_text(const char *value);
extern const char *get_var_slip_rr_text();
extern void set_var_slip_rr_text(const char *value);
extern const char *get_var_wd_mode_text();
extern void set_var_wd_mode_text(const char *value);
extern const char *get_var_wd_torque_text();
extern void set_var_wd_torque_text(const char *value);
extern bool get_var_is_tyre_warning();
extern void set_var_is_tyre_warning(bool value);
extern const char *get_var_ota_version_text();
extern void set_var_ota_version_text(const char *value);
extern const char *get_var_ota_status_text();
extern void set_var_ota_status_text(const char *value);
extern const char *get_var_ota_action_text();
extern void set_var_ota_action_text(const char *value);
extern bool get_var_is_ota_beta_channel();
extern void set_var_is_ota_beta_channel(bool value);
extern bool get_var_is_ota_stable_channel();
extern void set_var_is_ota_stable_channel(bool value);
extern bool get_var_is_ota_busy();
extern void set_var_is_ota_busy(bool value);
extern bool get_var_is_ota_error();
extern void set_var_is_ota_error(bool value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/