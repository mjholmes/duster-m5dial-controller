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
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *speedo;
    lv_obj_t *manual;
    lv_obj_t *speed_value;
    lv_obj_t *obj0;
    lv_obj_t *thermometer_label;
    lv_obj_t *temperature_value;
    lv_obj_t *mode_indicator;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *mode_indicator_1;
    lv_obj_t *led_bar;
    lv_obj_t *front_fog;
} objects_t;

extern objects_t objects;

void create_screen_speedo();
void tick_screen_speedo();

void create_screen_manual();
void tick_screen_manual();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/