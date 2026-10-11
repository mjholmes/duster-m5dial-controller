// EEZ native actions (components/ui/actions.h). Light changes go through
// control_request(); screen changes are local to the UI.

#include "actions.h"
#include "ui.h"
#include "control.h"

static void request_toggle(aux_light_id_t light)
{
    ctrl_req_t req = {
        .type = REQ_TOGGLE_LIGHT,
        .src = REQ_SRC_LOCAL_UI,
        .seq = control_next_seq(),
        .light = light,
    };
    control_request(&req);
}

void action_manualop_pressed_bar(lv_event_t *e)
{
    (void)e;
    request_toggle(AUX_LIGHT_LEDBAR);
}

void action_manualop_pressed_fog(lv_event_t *e)
{
    (void)e;
    request_toggle(AUX_LIGHT_FOG);
}

// Gesture/scroll events can repeat during one swipe; only start a load if the
// target is neither shown nor already loading.
static void switch_to(lv_obj_t *target, enum ScreensEnum id)
{
    lv_display_t *disp = lv_display_get_default();
    if (lv_display_get_screen_active(disp) == target || lv_display_get_screen_loading(disp) == target) {
        return;
    }
    // Stop the rest of this press (for example a swipe) acting on the new screen.
    lv_indev_t *indev = lv_indev_active();
    if (indev) {
        lv_indev_wait_release(indev);
    }
    loadScreen(id);
}

void action_switchto_screen_manual(lv_event_t *e)
{
    (void)e;
    switch_to(objects.manual, SCREEN_ID_MANUAL);
}

void action_switchto_screen_speedo(lv_event_t *e)
{
    (void)e;
    switch_to(objects.speedo, SCREEN_ID_SPEEDO);
}

// Screen ring (docs/ui-tyres-4wd-spec.md 10, supersedes per-screen gesture
// actions in SPEC 9.3): every screen's GESTURE event calls this one action.
// Adding a future screen means adding one row here.
static const struct { lv_obj_t **obj; enum ScreensEnum id; } s_nav_ring[] = {
    { &objects.speedo,          SCREEN_ID_SPEEDO },
    { &objects.manual,          SCREEN_ID_MANUAL },
    { &objects.tyre_pressures,  SCREEN_ID_TYRE_PRESSURES },
    { &objects.wheels_and_diff, SCREEN_ID_WHEELS_AND_DIFF },
};
#define NAV_RING_COUNT (int)(sizeof(s_nav_ring) / sizeof(s_nav_ring[0]))

void action_nav_gesture(lv_event_t *e)
{
    (void)e;
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    // OQ-28: swipe left = next screen, swipe right = previous; the ring
    // wraps at both ends; up/down (and no gesture) are ignored.
    int step;
    if (dir == LV_DIR_LEFT) {
        step = 1;
    } else if (dir == LV_DIR_RIGHT) {
        step = -1;
    } else {
        return;
    }

    lv_obj_t *active = lv_display_get_screen_active(lv_display_get_default());
    int current = -1;
    for (int i = 0; i < NAV_RING_COUNT; i++) {
        if (*s_nav_ring[i].obj == active) {
            current = i;
            break;
        }
    }
    if (current < 0) {
        return;  // active screen not in the ring: do nothing
    }

    int next = (current + step + NAV_RING_COUNT) % NAV_RING_COUNT;
    switch_to(*s_nav_ring[next].obj, s_nav_ring[next].id);
}

void action_switchto_screen_tyres(lv_event_t *e)
{
    (void)e;
    switch_to(objects.tyre_pressures, SCREEN_ID_TYRE_PRESSURES);
}

