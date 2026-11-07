#ifndef EEZ_LVGL_UI_FONTS_H
#define EEZ_LVGL_UI_FONTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t ui_font_saira_condensed_bold_92;
extern const lv_font_t ui_font_saira_condensed_bold_26;
extern const lv_font_t ui_font_saira_condensed_bold_20;
extern const lv_font_t ui_font_saira_condensed_bold_13;
extern const lv_font_t ui_font_fa_thermometer;
// On-device feedback: DTC codes and Engine readouts needed a size/glyph-set
// combination none of the EEZ-authored sizes had (each only embeds the
// glyphs its original design text used -- see components/ui/ui_font_*.c's
// own --symbols comment). Generated with lv_font_conv directly against the
// project's embedded Saira Condensed Bold TTF, matching EEZ Studio's own
// invocation (same tool, same --bpp/--format), so a real EEZ Studio Build
// can still coexist with these -- just don't let it regenerate over them.
extern const lv_font_t ui_font_dtc_code;
extern const lv_font_t ui_font_engine_value;

#ifndef EXT_FONT_DESC_T
#define EXT_FONT_DESC_T
typedef struct _ext_font_desc_t {
    const char *name;
    const void *font_ptr;
} ext_font_desc_t;
#endif

extern ext_font_desc_t fonts[];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_FONTS_H*/