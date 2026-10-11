#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: st_tyre_nodata
lv_style_t *get_style_st_tyre_nodata_MAIN_DEFAULT();
void add_style_st_tyre_nodata(lv_obj_t *obj);
void remove_style_st_tyre_nodata(lv_obj_t *obj);

// Style: st_tyre_ok
lv_style_t *get_style_st_tyre_ok_MAIN_DEFAULT();
void add_style_st_tyre_ok(lv_obj_t *obj);
void remove_style_st_tyre_ok(lv_obj_t *obj);

// Style: st_tyre_caution
lv_style_t *get_style_st_tyre_caution_MAIN_DEFAULT();
void add_style_st_tyre_caution(lv_obj_t *obj);
void remove_style_st_tyre_caution(lv_obj_t *obj);

// Style: st_tyre_alert
lv_style_t *get_style_st_tyre_alert_MAIN_DEFAULT();
void add_style_st_tyre_alert(lv_obj_t *obj);
void remove_style_st_tyre_alert(lv_obj_t *obj);

// Style: st_tyre_highlight
lv_style_t *get_style_st_tyre_highlight_MAIN_DEFAULT();
void add_style_st_tyre_highlight(lv_obj_t *obj);
void remove_style_st_tyre_highlight(lv_obj_t *obj);

// Style: st_coupler_ring_nodata
lv_style_t *get_style_st_coupler_ring_nodata_MAIN_DEFAULT();
void add_style_st_coupler_ring_nodata(lv_obj_t *obj);
void remove_style_st_coupler_ring_nodata(lv_obj_t *obj);

// Style: st_coupler_ring_open
lv_style_t *get_style_st_coupler_ring_open_MAIN_DEFAULT();
void add_style_st_coupler_ring_open(lv_obj_t *obj);
void remove_style_st_coupler_ring_open(lv_obj_t *obj);

// Style: st_coupler_ring_sliding
lv_style_t *get_style_st_coupler_ring_sliding_MAIN_DEFAULT();
void add_style_st_coupler_ring_sliding(lv_obj_t *obj);
void remove_style_st_coupler_ring_sliding(lv_obj_t *obj);

// Style: st_coupler_ring_closed
lv_style_t *get_style_st_coupler_ring_closed_MAIN_DEFAULT();
void add_style_st_coupler_ring_closed(lv_obj_t *obj);
void remove_style_st_coupler_ring_closed(lv_obj_t *obj);

// Style: st_coupler_fill_off
lv_style_t *get_style_st_coupler_fill_off_MAIN_DEFAULT();
void add_style_st_coupler_fill_off(lv_obj_t *obj);
void remove_style_st_coupler_fill_off(lv_obj_t *obj);

// Style: st_coupler_fill_sliding
lv_style_t *get_style_st_coupler_fill_sliding_MAIN_DEFAULT();
void add_style_st_coupler_fill_sliding(lv_obj_t *obj);
void remove_style_st_coupler_fill_sliding(lv_obj_t *obj);

// Style: st_coupler_fill_closed
lv_style_t *get_style_st_coupler_fill_closed_MAIN_DEFAULT();
void add_style_st_coupler_fill_closed(lv_obj_t *obj);
void remove_style_st_coupler_fill_closed(lv_obj_t *obj);

// Style: st_coupler_bar_off
lv_style_t *get_style_st_coupler_bar_off_MAIN_DEFAULT();
void add_style_st_coupler_bar_off(lv_obj_t *obj);
void remove_style_st_coupler_bar_off(lv_obj_t *obj);

// Style: st_coupler_bar_on
lv_style_t *get_style_st_coupler_bar_on_MAIN_DEFAULT();
void add_style_st_coupler_bar_on(lv_obj_t *obj);
void remove_style_st_coupler_bar_on(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/