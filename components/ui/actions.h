#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_manualop_pressed_bar(lv_event_t * e);
extern void action_manualop_pressed_fog(lv_event_t * e);
extern void action_switchto_screen_manual(lv_event_t * e);
extern void action_switchto_screen_speedo(lv_event_t * e);
extern void action_nav_gesture(lv_event_t * e);
extern void action_switchto_screen_tyres(lv_event_t * e);
extern void action_action_ota_select_stable(lv_event_t * e);
extern void action_action_ota_select_beta(lv_event_t * e);
extern void action_action_ota_button_pressed(lv_event_t * e);
extern void action_switchto_screen_dtc_codes(lv_event_t * e);
extern void action_switchto_screen_wifi_ota(lv_event_t * e);
extern void action_tyre_remap_start(lv_event_t * e);
extern void action_tyre_remap_confirm(lv_event_t * e);
extern void action_switchto_screen_settings(lv_event_t * e);
extern void action_switchto_screen_brightness(lv_event_t * e);
extern void action_select_brightness_day(lv_event_t * e);
extern void action_select_brightness_night(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/