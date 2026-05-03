/*!
    \file    themes/theme_terminal.c
    \brief   软装 A — TERMINAL：cursor 闪烁 + ASCII 边框 + 块字符 bar
*/

#include "themes.h"
#include <stdio.h>

void theme_terminal_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(style->bg), 0);

    /* === ASCII frame top === */
    lv_obj_t * top_bar = lv_label_create(scr);
    lv_obj_set_style_text_color(top_bar, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(top_bar, "+----[ system ]------------------------+");
    lv_obj_align(top_bar, LV_ALIGN_TOP_LEFT, 4, 4);

    /* === prompt + blinking cursor === */
    char prompt_buf[64];
    /* uptime_s % 2 → cursor visible / blank, ~1Hz blink */
    int blink_on = (st->uptime_s & 1) == 0;
    snprintf(prompt_buf, sizeof(prompt_buf), "%s%s",
             style->banner, blink_on ? " _" : "  ");
    lv_obj_t * prompt = lv_label_create(scr);
    lv_obj_set_style_text_color(prompt, lv_color_hex(style->accent), 0);
    lv_label_set_text(prompt, prompt_buf);
    lv_obj_align(prompt, LV_ALIGN_TOP_LEFT, 8, 22);

    /* === MAIN CLOCK === */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(style->fg_main), 0);
    lv_obj_set_style_text_letter_space(tlab, -1, 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_CENTER, 0, -8);

    /* === block-char battery bar (full width 20 cells) === */
    int bar_filled = (st->batt * 20 + 50) / 100; /* round */
    char bar[64] = "BAT [";
    int p = 5;
    /* use heavy/light shade ASCII ('#' + '-') — guaranteed in Montserrat */
    for (int i = 0; i < 20; i++) bar[p++] = (i < bar_filled) ? '#' : '-';
    snprintf(bar + p, sizeof(bar) - p, "] %3d%%", st->batt);
    lv_obj_t * bl = lv_label_create(scr);
    lv_obj_set_style_text_color(bl, lv_color_hex(style->accent), 0);
    lv_label_set_text(bl, bar);
    lv_obj_align(bl, LV_ALIGN_BOTTOM_LEFT, 8, -28);

    /* === stat row, fixed-width fields === */
    char stat[64];
    snprintf(stat, sizeof(stat), "hr %3d   steps %5d   up %4lus",
            st->hr, st->steps, (unsigned long)st->uptime_s);
    lv_obj_t * sl = lv_label_create(scr);
    lv_obj_set_style_text_color(sl, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(sl, stat);
    lv_obj_align(sl, LV_ALIGN_BOTTOM_LEFT, 8, -12);

    /* === bottom frame line === */
    lv_obj_t * bot_bar = lv_label_create(scr);
    lv_obj_set_style_text_color(bot_bar, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(bot_bar, "+--------------------------------------+");
    lv_obj_align(bot_bar, LV_ALIGN_BOTTOM_LEFT, 4, -2);
}
