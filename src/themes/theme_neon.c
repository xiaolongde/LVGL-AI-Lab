/*!
    \file    themes/theme_neon.c
    \brief   软装 D — NEON：双层 label 偏移模拟 neon 光晕；只读 .tstyle
*/

#include "themes.h"
#include <stdio.h>

/* helper: layered glow text — draws darker offset copies behind the bright one
   to fake a neon halo without any image. */
static void neon_text(lv_obj_t * scr, const char * text,
                      const lv_font_t * font,
                      uint32_t bright, uint32_t glow,
                      lv_align_t align, int32_t x, int32_t y)
{
    /* 4 cardinal-offset glow layers (left/right/up/down by 1 px) at 50% opa */
    static const int8_t off[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (int i = 0; i < 4; i++) {
        lv_obj_t * g = lv_label_create(scr);
        if (font) lv_obj_set_style_text_font(g, font, 0);
        lv_obj_set_style_text_color(g, lv_color_hex(glow), 0);
        lv_obj_set_style_text_opa(g, LV_OPA_50, 0);
        lv_label_set_text(g, text);
        lv_obj_align(g, align, x + off[i][0], y + off[i][1]);
    }
    /* bright top layer */
    lv_obj_t * t = lv_label_create(scr);
    if (font) lv_obj_set_style_text_font(t, font, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(bright), 0);
    lv_label_set_text(t, text);
    lv_obj_align(t, align, x, y);
}

void theme_neon_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(style->bg), 0);

    /* === BIG NEON CLOCK === */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    neon_text(scr, tbuf, font_pick(style->font_main),
              style->fg_main, style->accent,
              LV_ALIGN_CENTER, 0, -8);

    /* === sub neon === */
    neon_text(scr, style->banner, font_pick(style->font_secondary),
              style->fg_secondary, style->accent,
              LV_ALIGN_TOP_MID, 0, 14);

    neon_text(scr, style->sub_text, font_pick(style->font_secondary),
              style->fg_muted, style->accent,
              LV_ALIGN_BOTTOM_MID, 0, -12);

    /* === stat — single accent color === */
    char stat[40];
    snprintf(stat, sizeof(stat), "%d bpm   %d %%   %d steps",
             st->hr, st->batt, st->steps);
    lv_obj_t * sl = lv_label_create(scr);
    lv_obj_set_style_text_color(sl, lv_color_hex(style->accent), 0);
    lv_label_set_text(sl, stat);
    lv_obj_align(sl, LV_ALIGN_CENTER, 0, 26);
}
