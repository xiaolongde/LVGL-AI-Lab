/*!
    \file    themes/theme_zen.c
    \brief   软装 C — ZEN：时间被全圆 activity ring 环抱
*/

#include "themes.h"
#include <stdio.h>

static lv_color_t batt_tint(int batt, uint32_t base_accent)
{
    if (batt < 20) return lv_color_hex(0xFF453A); /* red */
    if (batt < 50) return lv_color_hex(0xE5A93F); /* amber */
    return lv_color_hex(base_accent);             /* rose gold */
}

void theme_zen_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(style->bg), 0);

    lv_color_t accent = batt_tint(st->batt, style->accent);

    /* === full activity ring centered on screen — wraps the clock === */
    lv_obj_t * arc = lv_arc_create(scr);
    lv_obj_set_size(arc, 150, 150);
    lv_arc_set_rotation(arc, 270);          /* 12 点位置起点 */
    lv_arc_set_bg_angles(arc, 0, 360);      /* 全圆 track */
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, st->batt);
    lv_obj_set_style_arc_width(arc, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_INDICATOR);   /* 加粗 indicator */
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x161616), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, accent, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    /* === clock — sits inside the ring === */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(style->fg_main), 0);
    lv_obj_set_style_text_letter_space(tlab, -3, 0); /* 戏剧性紧凑 */
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_CENTER, 0, -4);

    /* === battery % below clock, inside the ring === */
    char b[16];
    sprintf(b, "%d%%", st->batt);
    lv_obj_t * bl = lv_label_create(scr);
    lv_obj_set_style_text_color(bl, accent, 0);
    lv_obj_set_style_text_letter_space(bl, 1, 0);
    lv_label_set_text(bl, b);
    lv_obj_align(bl, LV_ALIGN_CENTER, 0, 22);

    /* === weekday top, far from ring === */
    lv_obj_t * day = lv_label_create(scr);
    lv_obj_set_style_text_color(day, lv_color_hex(style->fg_muted), 0);
    lv_obj_set_style_text_letter_space(day, 3, 0); /* 字符 breathe */
    lv_label_set_text(day, style->banner);
    lv_obj_align(day, LV_ALIGN_TOP_MID, 0, 6);

    /* === date bottom === */
    lv_obj_t * sub = lv_label_create(scr);
    lv_obj_set_style_text_color(sub, lv_color_hex(style->fg_muted), 0);
    lv_obj_set_style_text_letter_space(sub, 2, 0);
    lv_label_set_text(sub, style->sub_text);
    lv_obj_align(sub, LV_ALIGN_BOTTOM_MID, 0, -6);
}
