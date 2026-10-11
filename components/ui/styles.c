#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: st_tyre_nodata
//

void init_style_st_tyre_nodata_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x505660));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_tyre_nodata_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_tyre_nodata_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_tyre_nodata(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_tyre_nodata_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_tyre_nodata(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_tyre_nodata_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_tyre_ok
//

void init_style_st_tyre_ok_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x2eb45a));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_tyre_ok_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_tyre_ok_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_tyre_ok(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_tyre_ok_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_tyre_ok(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_tyre_ok_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_tyre_caution
//

void init_style_st_tyre_caution_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xf0aa28));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_tyre_caution_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_tyre_caution_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_tyre_caution(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_tyre_caution_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_tyre_caution(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_tyre_caution_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_tyre_alert
//

void init_style_st_tyre_alert_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xe63c32));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_tyre_alert_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_tyre_alert_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_tyre_alert(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_tyre_alert_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_tyre_alert(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_tyre_alert_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_tyre_highlight
//

void init_style_st_tyre_highlight_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_outline_width(style, 2);
    lv_style_set_outline_color(style, lv_color_hex(0xe63c32));
    lv_style_set_outline_opa(style, 255);
    lv_style_set_outline_pad(style, 1);
};

lv_style_t *get_style_st_tyre_highlight_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_tyre_highlight_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_tyre_highlight(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_tyre_highlight_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_tyre_highlight(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_tyre_highlight_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_ring_nodata
//

void init_style_st_coupler_ring_nodata_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_width(style, 3);
    lv_style_set_border_color(style, lv_color_hex(0x505660));
    lv_style_set_border_opa(style, 255);
};

lv_style_t *get_style_st_coupler_ring_nodata_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_ring_nodata_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_ring_nodata(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_ring_nodata_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_ring_nodata(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_ring_nodata_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_ring_open
//

void init_style_st_coupler_ring_open_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_width(style, 3);
    lv_style_set_border_color(style, lv_color_hex(0x6e7682));
    lv_style_set_border_opa(style, 255);
};

lv_style_t *get_style_st_coupler_ring_open_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_ring_open_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_ring_open(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_ring_open_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_ring_open(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_ring_open_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_ring_sliding
//

void init_style_st_coupler_ring_sliding_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_width(style, 3);
    lv_style_set_border_color(style, lv_color_hex(0xf0aa28));
    lv_style_set_border_opa(style, 255);
};

lv_style_t *get_style_st_coupler_ring_sliding_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_ring_sliding_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_ring_sliding(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_ring_sliding_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_ring_sliding(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_ring_sliding_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_ring_closed
//

void init_style_st_coupler_ring_closed_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_width(style, 3);
    lv_style_set_border_color(style, lv_color_hex(0x2eb45a));
    lv_style_set_border_opa(style, 255);
};

lv_style_t *get_style_st_coupler_ring_closed_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_ring_closed_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_ring_closed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_ring_closed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_ring_closed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_ring_closed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_fill_off
//

void init_style_st_coupler_fill_off_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
};

lv_style_t *get_style_st_coupler_fill_off_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_fill_off_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_fill_off(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_fill_off_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_fill_off(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_fill_off_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_fill_sliding
//

void init_style_st_coupler_fill_sliding_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xf0aa28));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_coupler_fill_sliding_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_fill_sliding_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_fill_sliding(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_fill_sliding_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_fill_sliding(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_fill_sliding_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_fill_closed
//

void init_style_st_coupler_fill_closed_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x2eb45a));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_coupler_fill_closed_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_fill_closed_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_fill_closed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_fill_closed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_fill_closed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_fill_closed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_bar_off
//

void init_style_st_coupler_bar_off_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
};

lv_style_t *get_style_st_coupler_bar_off_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_bar_off_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_bar_off(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_bar_off_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_bar_off(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_bar_off_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_coupler_bar_on
//

void init_style_st_coupler_bar_on_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0x6e7682));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_coupler_bar_on_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_coupler_bar_on_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_coupler_bar_on(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_coupler_bar_on_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_coupler_bar_on(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_coupler_bar_on_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_st_tyre_nodata,
        add_style_st_tyre_ok,
        add_style_st_tyre_caution,
        add_style_st_tyre_alert,
        add_style_st_tyre_highlight,
        add_style_st_coupler_ring_nodata,
        add_style_st_coupler_ring_open,
        add_style_st_coupler_ring_sliding,
        add_style_st_coupler_ring_closed,
        add_style_st_coupler_fill_off,
        add_style_st_coupler_fill_sliding,
        add_style_st_coupler_fill_closed,
        add_style_st_coupler_bar_off,
        add_style_st_coupler_bar_on,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_st_tyre_nodata,
        remove_style_st_tyre_ok,
        remove_style_st_tyre_caution,
        remove_style_st_tyre_alert,
        remove_style_st_tyre_highlight,
        remove_style_st_coupler_ring_nodata,
        remove_style_st_coupler_ring_open,
        remove_style_st_coupler_ring_sliding,
        remove_style_st_coupler_ring_closed,
        remove_style_st_coupler_fill_off,
        remove_style_st_coupler_fill_sliding,
        remove_style_st_coupler_fill_closed,
        remove_style_st_coupler_bar_off,
        remove_style_st_coupler_bar_on,
    };
    remove_style_funcs[styleIndex](obj);
}