/*******************************************************************************
 * Size: 13 px
 * Bpp: 4
 * Opts: --bpp 4 --size 13 --no-compress --font ../../Downloads/SairaCondensed-Bold.ttf --symbols mph --format lvgl
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl.h"
#endif

#ifndef UI_FONT_SAIRA_CONDENSED_BOLD_13
#define UI_FONT_SAIRA_CONDENSED_BOLD_13 1
#endif

#if UI_FONT_SAIRA_CONDENSED_BOLD_13

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0068 "h" */
    0x6f, 0x40, 0x0, 0x6f, 0x40, 0x0, 0x6f, 0x40,
    0x0, 0x6f, 0xae, 0xc0, 0x6f, 0xcd, 0xf3, 0x6f,
    0x46, 0xf4, 0x6f, 0x46, 0xf4, 0x6f, 0x46, 0xf4,
    0x6f, 0x46, 0xf4, 0x6f, 0x46, 0xf4,

    /* U+006D "m" */
    0x6f, 0x9e, 0xc8, 0xfa, 0x6, 0xfb, 0xcf, 0xbd,
    0xf0, 0x6f, 0x48, 0xf3, 0x9f, 0x16, 0xf4, 0x7f,
    0x39, 0xf1, 0x6f, 0x47, 0xf3, 0x9f, 0x16, 0xf4,
    0x7f, 0x39, 0xf1, 0x6f, 0x47, 0xf3, 0x9f, 0x10,

    /* U+0070 "p" */
    0x6f, 0x9f, 0xb0, 0x6f, 0xbb, 0xf3, 0x6f, 0x45,
    0xf5, 0x6f, 0x45, 0xf5, 0x6f, 0x45, 0xf5, 0x6f,
    0xcc, 0xf3, 0x6f, 0xae, 0xc0, 0x6f, 0x40, 0x0,
    0x6f, 0x40, 0x0, 0x6f, 0x40, 0x0
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 94, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 139, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x5, 0x8
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 104, .range_length = 9, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_saira_condensed_bold_13 = {
#else
lv_font_t ui_font_saira_condensed_bold_13 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};

#endif /*#if UI_FONT_SAIRA_CONDENSED_BOLD_13*/