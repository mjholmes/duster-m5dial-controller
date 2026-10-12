#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_SPEEDO = 1,
    SCREEN_ID_MANUAL = 2,
    SCREEN_ID_TYRE_PRESSURES = 3,
    SCREEN_ID_WHEELS_AND_DIFF = 4,
    SCREEN_ID_WI_FI_OTA = 5,
    _SCREEN_ID_LAST = 5
};

typedef struct _objects_t {
    lv_obj_t *speedo;
    lv_obj_t *manual;
    lv_obj_t *tyre_pressures;
    lv_obj_t *wheels_and_diff;
    lv_obj_t *wi_fi_ota;
    lv_obj_t *speed_value;
    lv_obj_t *obj0;
    lv_obj_t *thermometer_label;
    lv_obj_t *temperature_value;
    lv_obj_t *mode_indicator;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *dtc_warn_icon;
    lv_obj_t *dtccount;
    lv_obj_t *obj3;
    lv_obj_t *comms;
    lv_obj_t *tyre_warn_icon;
    lv_obj_t *mode_indicator_1;
    lv_obj_t *led_bar;
    lv_obj_t *front_fog;
    lv_obj_t *coupler;
    lv_obj_t *coupler_fill;
    lv_obj_t *coupler_bar;
    lv_obj_t *tyre_fl;
    lv_obj_t *tyre_fr;
    lv_obj_t *tyre_rl;
    lv_obj_t *tyre_rr;
    lv_obj_t *tyre_fl_value;
    lv_obj_t *tyre_fl_unit;
    lv_obj_t *tyre_fr_value;
    lv_obj_t *tyre_fr_unit;
    lv_obj_t *tyre_rl_value;
    lv_obj_t *tyre_rl_unit;
    lv_obj_t *tyre_rr_value;
    lv_obj_t *tyre_rr_unit;
    lv_obj_t *tyres_header;
    lv_obj_t *wd_coupler;
    lv_obj_t *wd_coupler_fill;
    lv_obj_t *wd_coupler_bar;
    lv_obj_t *wd_tyre_fl;
    lv_obj_t *wd_tyre_fr;
    lv_obj_t *wd_tyre_rl;
    lv_obj_t *wd_tyre_rr;
    lv_obj_t *wd_slip_fl_value;
    lv_obj_t *wd_slip_fl_unit;
    lv_obj_t *wd_slip_fr_value;
    lv_obj_t *wd_slip_fr_unit;
    lv_obj_t *wd_slip_rl_value;
    lv_obj_t *wd_slip_rl_unit;
    lv_obj_t *wd_slip_rr_value;
    lv_obj_t *wd_slip_rr_unit;
    lv_obj_t *wd_mode;
    lv_obj_t *wd_torque;
    lv_obj_t *ota_version_text;
    lv_obj_t *ota_select_stable;
    lv_obj_t *ota_select_beta;
    lv_obj_t *ota_status_text;
    lv_obj_t *ota_action_button;
    lv_obj_t *obj4;
} objects_t;

extern objects_t objects;

void create_screen_speedo();
void tick_screen_speedo();

void create_screen_manual();
void tick_screen_manual();

void create_screen_tyre_pressures();
void tick_screen_tyre_pressures();

void create_screen_wheels_and_diff();
void tick_screen_wheels_and_diff();

void create_screen_wi_fi_ota();
void tick_screen_wi_fi_ota();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/