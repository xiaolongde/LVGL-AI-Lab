/*!
    \file    themes/theme_pixel.c
    \brief   软装 B — PIXEL：accent dot 带光晕 + thin 分隔线 + 字距统一
*/

#include "themes.h"
#include <stdio.h>

void theme_pixel_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(style->bg), 0);

    /* === top-left date === */
    lv_obj_t * day = lv_label_create(scr);
    lv_obj_set_style_text_color(day, lv_color_hex(style->fg_secondary), 0);
    lv_obj_set_style_text_letter_space(day, 1, 0);
    lv_label_set_text(day, style->sub_text);
    lv_obj_align(day, LV_ALIGN_TOP_LEFT, 16, 14);

    /* === top-right battery (dynamic color) === */
    char bb[16];
    sprintf(bb, LV_SYMBOL_BATTERY_FULL "  %d%%", st->batt);
    uint32_t bcolor = (st->batt < 20) ? 0xFF453A : style->accent;
    lv_obj_t * bat = lv_label_create(scr);
    lv_obj_set_style_text_color(bat, lv_color_hex(bcolor), 0);
    lv_label_set_text(bat, bb);
    lv_obj_align(bat, LV_ALIGN_TOP_RIGHT, -16, 14);

    /* === main clock left === */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    lv_obj_set_style_text_font(tlab, font_pick(style->font_main), 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(style->fg_main), 0);
    lv_obj_set_style_text_letter_space(tlab, -2, 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_LEFT_MID, 16, -8);

    /* === accent dot with halo (2 stacked obj — outer dim, inner bright) === */
    lv_obj_t * halo = lv_obj_create(scr);
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, 12, 12);
    lv_obj_set_pos(halo, 19, 124);
    lv_obj_set_style_bg_color(halo, lv_color_hex(style->accent), 0);
    lv_obj_set_style_bg_opa(halo, LV_OPA_30, 0);
    lv_obj_set_style_radius(halo, 6, 0);
    lv_obj_clear_flag(halo, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * dot = lv_obj_create(scr);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_pos(dot, 22, 127);
    lv_obj_set_style_bg_color(dot, lv_color_hex(style->accent), 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dot, 3, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

    /* === thin vertical separator between time half and stat half === */
    lv_obj_t * sep = lv_obj_create(scr);
    lv_obj_remove_style_all(sep);
    lv_obj_set_size(sep, 1, 80);
    lv_obj_set_pos(sep, 158, 46);
    lv_obj_set_style_bg_color(sep, lv_color_hex(style->fg_secondary), 0);
    lv_obj_set_style_bg_opa(sep, LV_OPA_30, 0);
    lv_obj_clear_flag(sep, LV_OBJ_FLAG_SCROLLABLE);

    /* === right column stats — uniform spacing, accent letter === */
    const lv_font_t * font_sec = font_pick(style->font_secondary);

    char l1[24], l2[24], l3[24];
    snprintf(l1, sizeof(l1), "%5d", st->steps);
    snprintf(l2, sizeof(l2), "%3d bpm", st->hr);
    snprintf(l3, sizeof(l3), "up %lus", (unsigned long)st->uptime_s);

    lv_obj_t * sl1 = lv_label_create(scr);
    lv_obj_set_style_text_font(sl1, font_sec, 0);
    lv_obj_set_style_text_color(sl1, lv_color_hex(style->fg_main), 0);
    lv_label_set_text(sl1, l1);
    lv_obj_align(sl1, LV_ALIGN_RIGHT_MID, -16, -22);

    lv_obj_t * lbl1 = lv_label_create(scr);
    lv_obj_set_style_text_color(lbl1, lv_color_hex(style->fg_secondary), 0);
    lv_obj_set_style_text_letter_space(lbl1, 1, 0);
    lv_label_set_text(lbl1, "STEPS");
    lv_obj_align(lbl1, LV_ALIGN_RIGHT_MID, -16, -42);

    lv_obj_t * sl2 = lv_label_create(scr);
    lv_obj_set_style_text_font(sl2, font_sec, 0);
    lv_obj_set_style_text_color(sl2, lv_color_hex(style->fg_main), 0);
    lv_label_set_text(sl2, l2);
    lv_obj_align(sl2, LV_ALIGN_RIGHT_MID, -16, 8);

    lv_obj_t * sl3 = lv_label_create(scr);
    lv_obj_set_style_text_color(sl3, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(sl3, l3);
    lv_obj_align(sl3, LV_ALIGN_RIGHT_MID, -16, 28);
}
