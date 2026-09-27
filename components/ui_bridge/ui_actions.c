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

