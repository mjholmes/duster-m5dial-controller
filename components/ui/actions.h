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

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/