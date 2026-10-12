#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_speedo_thermometer_label(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_frost_risk(value);
        }
    }
}

static void event_handler_cb_speedo_temperature_value(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_below_freezing(value);
        }
    }
}

static void event_handler_cb_speedo_mode_indicator(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_auto_mode(value);
        }
    }
}

static void event_handler_cb_speedo_obj1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_opstatus_ledbar(value);
        }
    }
}

static void event_handler_cb_speedo_obj2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_opstatus_fog(value);
        }
    }
}

static void event_handler_cb_speedo_dtc_warn_icon(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_dtc_present(value);
        }
    }
}

static void event_handler_cb_speedo_dtccount(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_dtc_present(value);
        }
    }
}

static void event_handler_cb_speedo_obj3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_comms_lost(value);
        }
    }
}

static void event_handler_cb_speedo_comms(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_comms_lost(value);
        }
    }
}

static void event_handler_cb_speedo_tyre_warn_icon(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_tyre_warning(value);
        }
    }
}

static void event_handler_cb_manual_mode_indicator_1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_auto_mode(value);
        }
    }
}

static void event_handler_cb_manual_led_bar(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_opstatus_ledbar(value);
        }
    }
}

static void event_handler_cb_manual_front_fog(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_opstatus_fog(value);
        }
    }
}

static void event_handler_cb_wi_fi_ota_ota_select_stable(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_ota_stable_channel(value);
        }
    }
}

static void event_handler_cb_wi_fi_ota_ota_select_beta(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_ota_beta_channel(value);
        }
    }
}

static void event_handler_cb_wi_fi_ota_ota_status_text(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            set_var_is_ota_error(value);
        }
    }
}

//
// Screens
//

void create_screen_speedo() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.speedo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, action_nav_gesture, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed_value = obj;
            lv_obj_set_pos(obj, 63, 77);
            lv_obj_set_size(obj, 114, 74);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_92, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 105, 148);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_EDITED);
            lv_label_set_text_static(obj, "mph");
        }
        {
            // thermometer_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.thermometer_label = obj;
            lv_obj_set_pos(obj, 86, 186);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_thermometer_label, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_fa_thermometer, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00bcd4), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text_static(obj, "");
        }
        {
            // temperature_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.temperature_value = obj;
            lv_obj_set_pos(obj, 109, 187);
            lv_obj_set_size(obj, 46, 19);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_temperature_value, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_26, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00bcd4), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "");
        }
        {
            // mode_indicator
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.mode_indicator = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 240);
            lv_arc_set_range(obj, 0, 4);
            lv_arc_set_value(obj, 4);
            lv_arc_set_bg_start_angle(obj, 0);
            lv_arc_set_bg_end_angle(obj, 360);
            lv_arc_set_rotation(obj, 225);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_mode_indicator, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0x00bd14), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xff9800), LV_PART_INDICATOR | LV_STATE_CHECKED);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 26, 83);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_main_beam);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_obj1, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_image_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 23, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_front_fog);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_obj2, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_image_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dtc_warn_icon
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.dtc_warn_icon = obj;
            lv_obj_set_pos(obj, 94, 35);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_warning);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_dtc_warn_icon, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_image_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dtccount
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dtccount = obj;
            lv_obj_set_pos(obj, 121, 39);
            lv_obj_set_size(obj, 31, 18);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_dtccount, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_EDITED);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 184, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_comms);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_obj3, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_image_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // comms
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.comms = obj;
            lv_obj_set_pos(obj, 177, 129);
            lv_obj_set_size(obj, 45, 18);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_comms, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_EDITED);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "");
        }
        {
            // tyre_warn_icon
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.tyre_warn_icon = obj;
            lv_obj_set_pos(obj, 172, 150);
            lv_obj_set_size(obj, 44, 44);
            lv_image_set_src(obj, &img_tyre_warning);
            lv_obj_add_event_cb(obj, action_switchto_screen_tyres, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_speedo_tyre_warn_icon, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_CHECKED);
        }
    }
    
    tick_screen_speedo();
}

void tick_screen_speedo() {
    {
        const char *new_val = get_var_speed();
        const char *cur_val = lv_label_get_text(objects.speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.speed_value;
            lv_label_set_text(objects.speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_frost_risk();
        bool cur_val = lv_obj_has_state(objects.thermometer_label, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.thermometer_label;
            if (new_val) {
                lv_obj_add_state(objects.thermometer_label, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.thermometer_label, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_temperature();
        const char *cur_val = lv_label_get_text(objects.temperature_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.temperature_value;
            lv_label_set_text(objects.temperature_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_below_freezing();
        bool cur_val = lv_obj_has_state(objects.temperature_value, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.temperature_value;
            if (new_val) {
                lv_obj_add_state(objects.temperature_value, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.temperature_value, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_auto_mode();
        bool cur_val = lv_obj_has_state(objects.mode_indicator, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mode_indicator;
            if (new_val) {
                lv_obj_add_state(objects.mode_indicator, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.mode_indicator, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_opstatus_ledbar();
        bool cur_val = lv_obj_has_state(objects.obj1, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.obj1;
            if (new_val) {
                lv_obj_add_state(objects.obj1, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.obj1, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_opstatus_fog();
        bool cur_val = lv_obj_has_state(objects.obj2, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.obj2;
            if (new_val) {
                lv_obj_add_state(objects.obj2, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.obj2, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_dtc_present();
        bool cur_val = lv_obj_has_state(objects.dtc_warn_icon, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.dtc_warn_icon;
            if (new_val) {
                lv_obj_add_state(objects.dtc_warn_icon, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.dtc_warn_icon, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_dtc_count();
        const char *cur_val = lv_label_get_text(objects.dtccount);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.dtccount;
            lv_label_set_text(objects.dtccount, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_dtc_present();
        bool cur_val = lv_obj_has_state(objects.dtccount, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.dtccount;
            if (new_val) {
                lv_obj_add_state(objects.dtccount, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.dtccount, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_comms_lost();
        bool cur_val = lv_obj_has_state(objects.obj3, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.obj3;
            if (new_val) {
                lv_obj_add_state(objects.obj3, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.obj3, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_comms_text();
        const char *cur_val = lv_label_get_text(objects.comms);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.comms;
            lv_label_set_text(objects.comms, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_comms_lost();
        bool cur_val = lv_obj_has_state(objects.comms, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.comms;
            if (new_val) {
                lv_obj_add_state(objects.comms, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.comms, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_tyre_warning();
        bool cur_val = lv_obj_has_flag(objects.tyre_warn_icon, LV_OBJ_FLAG_CLICKABLE);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.tyre_warn_icon;
            if (new_val) {
                lv_obj_add_flag(objects.tyre_warn_icon, LV_OBJ_FLAG_CLICKABLE);
            } else {
                lv_obj_remove_flag(objects.tyre_warn_icon, LV_OBJ_FLAG_CLICKABLE);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_tyre_warning();
        bool cur_val = lv_obj_has_state(objects.tyre_warn_icon, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.tyre_warn_icon;
            if (new_val) {
                lv_obj_add_state(objects.tyre_warn_icon, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.tyre_warn_icon, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_manual() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.manual = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, action_nav_gesture, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // mode_indicator_1
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.mode_indicator_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 240, 240);
            lv_arc_set_range(obj, 0, 4);
            lv_arc_set_value(obj, 4);
            lv_arc_set_bg_start_angle(obj, 0);
            lv_arc_set_bg_end_angle(obj, 360);
            lv_arc_set_rotation(obj, 225);
            lv_obj_add_event_cb(obj, event_handler_cb_manual_mode_indicator_1, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0x00bd14), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xff9800), LV_PART_INDICATOR | LV_STATE_CHECKED);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // LED Bar
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.led_bar = obj;
            lv_obj_set_pos(obj, 53, 39);
            lv_obj_set_size(obj, 134, 70);
            lv_obj_add_event_cb(obj, action_manualop_pressed_bar, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_manual_led_bar, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x00bd14), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "LED Bar");
                }
            }
        }
        {
            // Front Fog
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.front_fog = obj;
            lv_obj_set_pos(obj, 53, 128);
            lv_obj_set_size(obj, 134, 70);
            lv_obj_add_event_cb(obj, action_manualop_pressed_fog, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_manual_front_fog, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff9800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x00bd14), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Fog");
                }
            }
        }
    }
    
    tick_screen_manual();
}

void tick_screen_manual() {
    {
        bool new_val = get_var_is_auto_mode();
        bool cur_val = lv_obj_has_state(objects.mode_indicator_1, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.mode_indicator_1;
            if (new_val) {
                lv_obj_add_state(objects.mode_indicator_1, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.mode_indicator_1, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_opstatus_ledbar();
        bool cur_val = lv_obj_has_state(objects.led_bar, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_bar;
            if (new_val) {
                lv_obj_add_state(objects.led_bar, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.led_bar, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_auto_mode();
        bool cur_val = lv_obj_has_state(objects.led_bar, LV_STATE_DISABLED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_bar;
            if (new_val) {
                lv_obj_add_state(objects.led_bar, LV_STATE_DISABLED);
            } else {
                lv_obj_remove_state(objects.led_bar, LV_STATE_DISABLED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_opstatus_fog();
        bool cur_val = lv_obj_has_state(objects.front_fog, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.front_fog;
            if (new_val) {
                lv_obj_add_state(objects.front_fog, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.front_fog, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_auto_mode();
        bool cur_val = lv_obj_has_state(objects.front_fog, LV_STATE_DISABLED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.front_fog;
            if (new_val) {
                lv_obj_add_state(objects.front_fog, LV_STATE_DISABLED);
            } else {
                lv_obj_remove_state(objects.front_fog, LV_STATE_DISABLED);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_tyre_pressures() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.tyre_pressures = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, action_nav_gesture, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 79, 32);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_chassis);
        }
        {
            // coupler
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.coupler = obj;
            lv_obj_set_pos(obj, 110, 134);
            lv_obj_set_size(obj, 20, 20);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_ring_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // coupler_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.coupler_fill = obj;
            lv_obj_set_pos(obj, 116, 140);
            lv_obj_set_size(obj, 8, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_fill_off(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // coupler_bar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.coupler_bar = obj;
            lv_obj_set_pos(obj, 115, 143);
            lv_obj_set_size(obj, 10, 2);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_bar_off(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tyre_fl
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tyre_fl = obj;
            lv_obj_set_pos(obj, 70, 58);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tyre_fr
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tyre_fr = obj;
            lv_obj_set_pos(obj, 154, 58);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tyre_rl
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tyre_rl = obj;
            lv_obj_set_pos(obj, 70, 156);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tyre_rr
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tyre_rr = obj;
            lv_obj_set_pos(obj, 154, 156);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tyre_fl_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_fl_value = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // tyre_fl_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_fl_unit = obj;
            lv_obj_set_pos(obj, 24, 85);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "psi");
        }
        {
            // tyre_fr_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_fr_value = obj;
            lv_obj_set_pos(obj, 176, 64);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // tyre_fr_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_fr_unit = obj;
            lv_obj_set_pos(obj, 176, 85);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "psi");
        }
        {
            // tyre_rl_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_rl_value = obj;
            lv_obj_set_pos(obj, 24, 162);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // tyre_rl_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_rl_unit = obj;
            lv_obj_set_pos(obj, 24, 183);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "psi");
        }
        {
            // tyre_rr_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_rr_value = obj;
            lv_obj_set_pos(obj, 176, 162);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // tyre_rr_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyre_rr_unit = obj;
            lv_obj_set_pos(obj, 176, 183);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "psi");
        }
        {
            // tyres_header
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tyres_header = obj;
            lv_obj_set_pos(obj, 70, 12);
            lv_obj_set_size(obj, 100, 18);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "TYRES");
        }
    }
    
    tick_screen_tyre_pressures();
}

void tick_screen_tyre_pressures() {
    {
        const char *new_val = get_var_tyre_fl_text();
        const char *cur_val = lv_label_get_text(objects.tyre_fl_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tyre_fl_value;
            lv_label_set_text(objects.tyre_fl_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_tyre_fr_text();
        const char *cur_val = lv_label_get_text(objects.tyre_fr_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tyre_fr_value;
            lv_label_set_text(objects.tyre_fr_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_tyre_rl_text();
        const char *cur_val = lv_label_get_text(objects.tyre_rl_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tyre_rl_value;
            lv_label_set_text(objects.tyre_rl_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_tyre_rr_text();
        const char *cur_val = lv_label_get_text(objects.tyre_rr_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tyre_rr_value;
            lv_label_set_text(objects.tyre_rr_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_wheels_and_diff() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wheels_and_diff = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, action_nav_gesture, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 79, 32);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_chassis);
        }
        {
            // wd_coupler
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_coupler = obj;
            lv_obj_set_pos(obj, 110, 134);
            lv_obj_set_size(obj, 20, 20);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_ring_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_coupler_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_coupler_fill = obj;
            lv_obj_set_pos(obj, 116, 140);
            lv_obj_set_size(obj, 8, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_fill_off(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_coupler_bar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_coupler_bar = obj;
            lv_obj_set_pos(obj, 115, 143);
            lv_obj_set_size(obj, 10, 2);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_coupler_bar_off(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_tyre_fl
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_tyre_fl = obj;
            lv_obj_set_pos(obj, 70, 58);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_tyre_fr
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_tyre_fr = obj;
            lv_obj_set_pos(obj, 154, 58);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_tyre_rl
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_tyre_rl = obj;
            lv_obj_set_pos(obj, 70, 156);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_tyre_rr
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wd_tyre_rr = obj;
            lv_obj_set_pos(obj, 154, 156);
            lv_obj_set_size(obj, 16, 36);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            add_style_st_tyre_nodata(obj);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // wd_slip_fl_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_fl_value = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // wd_slip_fl_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_fl_unit = obj;
            lv_obj_set_pos(obj, 24, 85);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "slip");
        }
        {
            // wd_slip_fr_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_fr_value = obj;
            lv_obj_set_pos(obj, 176, 64);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // wd_slip_fr_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_fr_unit = obj;
            lv_obj_set_pos(obj, 176, 85);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "slip");
        }
        {
            // wd_slip_rl_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_rl_value = obj;
            lv_obj_set_pos(obj, 24, 162);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // wd_slip_rl_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_rl_unit = obj;
            lv_obj_set_pos(obj, 24, 183);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "slip");
        }
        {
            // wd_slip_rr_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_rr_value = obj;
            lv_obj_set_pos(obj, 176, 162);
            lv_obj_set_size(obj, 40, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // wd_slip_rr_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_slip_rr_unit = obj;
            lv_obj_set_pos(obj, 176, 183);
            lv_obj_set_size(obj, 40, 16);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "slip");
        }
        {
            // wd_mode
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_mode = obj;
            lv_obj_set_pos(obj, 70, 12);
            lv_obj_set_size(obj, 100, 18);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // wd_torque
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wd_torque = obj;
            lv_obj_set_pos(obj, 60, 206);
            lv_obj_set_size(obj, 120, 24);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x697380), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_wheels_and_diff();
}

void tick_screen_wheels_and_diff() {
    {
        const char *new_val = get_var_slip_fl_text();
        const char *cur_val = lv_label_get_text(objects.wd_slip_fl_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_slip_fl_value;
            lv_label_set_text(objects.wd_slip_fl_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_slip_fr_text();
        const char *cur_val = lv_label_get_text(objects.wd_slip_fr_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_slip_fr_value;
            lv_label_set_text(objects.wd_slip_fr_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_slip_rl_text();
        const char *cur_val = lv_label_get_text(objects.wd_slip_rl_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_slip_rl_value;
            lv_label_set_text(objects.wd_slip_rl_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_slip_rr_text();
        const char *cur_val = lv_label_get_text(objects.wd_slip_rr_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_slip_rr_value;
            lv_label_set_text(objects.wd_slip_rr_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_wd_mode_text();
        const char *cur_val = lv_label_get_text(objects.wd_mode);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_mode;
            lv_label_set_text(objects.wd_mode, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_wd_torque_text();
        const char *cur_val = lv_label_get_text(objects.wd_torque);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.wd_torque;
            lv_label_set_text(objects.wd_torque, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_wi_fi_ota() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wi_fi_ota = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, action_nav_gesture, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ota_version_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ota_version_text = obj;
            lv_obj_set_pos(obj, 70, 12);
            lv_obj_set_size(obj, 100, 18);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ota_select_stable
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ota_select_stable = obj;
            lv_obj_set_pos(obj, 40, 36);
            lv_obj_set_size(obj, 70, 28);
            lv_obj_add_event_cb(obj, action_action_ota_select_stable, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_wi_fi_ota_ota_select_stable, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x00bd14), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x505660), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Stable");
                }
            }
        }
        {
            // ota_select_beta
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ota_select_beta = obj;
            lv_obj_set_pos(obj, 130, 36);
            lv_obj_set_size(obj, 70, 28);
            lv_obj_add_event_cb(obj, action_action_ota_select_beta, LV_EVENT_CLICKED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_cb_wi_fi_ota_ota_select_beta, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x00bd14), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x505660), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Beta");
                }
            }
        }
        {
            // ota_status_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ota_status_text = obj;
            lv_obj_set_pos(obj, 45, 88);
            lv_obj_set_size(obj, 150, 56);
            lv_obj_add_event_cb(obj, event_handler_cb_wi_fi_ota_ota_status_text, LV_EVENT_ALL, 0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_saira_condensed_bold_13, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe63c32), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_label_set_text(obj, "");
        }
        {
            // ota_action_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ota_action_button = obj;
            lv_obj_set_pos(obj, 70, 160);
            lv_obj_set_size(obj, 100, 36);
            lv_obj_add_event_cb(obj, action_action_ota_button_pressed, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x00bd14), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj4 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
    }
    
    tick_screen_wi_fi_ota();
}

void tick_screen_wi_fi_ota() {
    {
        const char *new_val = get_var_ota_version_text();
        const char *cur_val = lv_label_get_text(objects.ota_version_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ota_version_text;
            lv_label_set_text(objects.ota_version_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_stable_channel();
        bool cur_val = lv_obj_has_state(objects.ota_select_stable, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_select_stable;
            if (new_val) {
                lv_obj_add_state(objects.ota_select_stable, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.ota_select_stable, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_busy();
        bool cur_val = lv_obj_has_state(objects.ota_select_stable, LV_STATE_DISABLED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_select_stable;
            if (new_val) {
                lv_obj_add_state(objects.ota_select_stable, LV_STATE_DISABLED);
            } else {
                lv_obj_remove_state(objects.ota_select_stable, LV_STATE_DISABLED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_beta_channel();
        bool cur_val = lv_obj_has_state(objects.ota_select_beta, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_select_beta;
            if (new_val) {
                lv_obj_add_state(objects.ota_select_beta, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.ota_select_beta, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_busy();
        bool cur_val = lv_obj_has_state(objects.ota_select_beta, LV_STATE_DISABLED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_select_beta;
            if (new_val) {
                lv_obj_add_state(objects.ota_select_beta, LV_STATE_DISABLED);
            } else {
                lv_obj_remove_state(objects.ota_select_beta, LV_STATE_DISABLED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ota_status_text();
        const char *cur_val = lv_label_get_text(objects.ota_status_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ota_status_text;
            lv_label_set_text(objects.ota_status_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_error();
        bool cur_val = lv_obj_has_state(objects.ota_status_text, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_status_text;
            if (new_val) {
                lv_obj_add_state(objects.ota_status_text, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.ota_status_text, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_is_ota_busy();
        bool cur_val = lv_obj_has_state(objects.ota_action_button, LV_STATE_DISABLED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.ota_action_button;
            if (new_val) {
                lv_obj_add_state(objects.ota_action_button, LV_STATE_DISABLED);
            } else {
                lv_obj_remove_state(objects.ota_action_button, LV_STATE_DISABLED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ota_action_text();
        const char *cur_val = lv_label_get_text(objects.obj4);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.obj4;
            lv_label_set_text(objects.obj4, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_speedo,
    tick_screen_manual,
    tick_screen_tyre_pressures,
    tick_screen_wheels_and_diff,
    tick_screen_wi_fi_ota,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 5) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "SairaCondensedBold_92", &ui_font_saira_condensed_bold_92 },
    { "SairaCondensedBold_26", &ui_font_saira_condensed_bold_26 },
    { "SairaCondensedBold_20", &ui_font_saira_condensed_bold_20 },
    { "SairaCondensedBold_13", &ui_font_saira_condensed_bold_13 },
    { "fa-thermometer", &ui_font_fa_thermometer },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_speedo();
    create_screen_manual();
    create_screen_tyre_pressures();
    create_screen_wheels_and_diff();
    create_screen_wi_fi_ota();
}