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
    FLOW_GLOBAL_VARIABLE_OPSTATUS_LEDBAR = 6
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

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/