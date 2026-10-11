// Named-style swapping for the tyre/4WD screens (docs/ui-tyres-4wd-spec.md
// 1, 6.2-6.3). ui task only -- touches lv_obj_t*, not host-testable; the
// severity/state logic it reads is tested via tyre_logic.c instead.
#pragma once

// Call once per ui loop, after ui_vars_refresh(). Swaps each tracked
// object's named style only on a state change.
void ui_styles_update(void);
