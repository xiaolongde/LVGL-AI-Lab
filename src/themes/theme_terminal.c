/*!
    \file    themes/theme_terminal.c
    \brief   软装 A — TERMINAL：fake shell 滚动 history + block cursor + 装饰
*/

#include "themes.h"
#include <stdio.h>

/* small fake shell log — cycles based on uptime_s for "活的 terminal" 感 */
static const char * SHELL_LINES[] = {
    "$ ls /sys/power",
    "battery   thermal   wakeup",
    "$ cat /proc/heart",
    "rate=72 bpm  hr_var=ok",
    "$ uptime",
    "load: 0.42  steps:8423",
    "$ tail -f /var/log/wear",
    "[ok] watchface=zen",
    "$ neofetch --tiny",
    "  os=watchOS-mock 0.3.1",
    "$ free -h",
    "  ram 30K/48K  flash 251K/256K",
};
#define SHELL_N ((int)(sizeof(SHELL_LINES) / sizeof(SHELL_LINES[0])))

void theme_terminal_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(style->bg), 0);

    /* === ASCII frame top === */
    lv_obj_t * top_bar = lv_label_create(scr);
    lv_obj_set_style_text_color(top_bar, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(top_bar, "+----[ system ]------------------------+");
    lv_obj_align(top_bar, LV_ALIGN_TOP_LEFT, 4, 4);

    /* === scrolling fake shell history (3 lines visible) === */
    int base = (int)(st->uptime_s / 2) % SHELL_N;
    for (int i = 0; i < 3; i++) {
        const char * line = SHELL_LINES[(base + i) % SHELL_N];
        lv_obj_t * lab = lv_label_create(scr);
        /* shell prompt lines (start with $) get accent; output is muted */
        uint32_t color = (line[0] == '$')
            ? style->accent
            : style->fg_secondary;
        lv_obj_set_style_text_color(lab, lv_color_hex(color), 0);
        lv_label_set_text(lab, line);
        lv_obj_align(lab, LV_ALIGN_TOP_LEFT, 8, 22 + i * 14);
    }

    /* === current prompt with block cursor (1Hz blink via uptime_s parity) === */
    char prompt_buf[64];
    int blink_on = (st->uptime_s & 1) == 0;
    snprintf(prompt_buf, sizeof(prompt_buf), "$ %s",
             blink_on ? "\xE2\x96\x88" : " "); /* █ U+2588 — falls back to ' ' if font lacks it */
    lv_obj_t * prompt = lv_label_create(scr);
    lv_obj_set_style_text_color(prompt, lv_color_hex(style->fg_main), 0);
    lv_label_set_text(prompt, prompt_buf);
    lv_obj_align(prompt, LV_ALIGN_TOP_LEFT, 8, 22 + 3 * 14);

    /* === clock — overlay center as the focal point === */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(style->fg_main), 0);
    lv_obj_set_style_text_letter_space(tlab, -1, 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_RIGHT_MID, -16, -8);

    /* === bar === */
    int bar_filled = (st->batt * 18 + 50) / 100;
    char bar[48] = "BAT [";
    int p = 5;
    for (int i = 0; i < 18; i++) bar[p++] = (i < bar_filled) ? '#' : '-';
    snprintf(bar + p, sizeof(bar) - p, "] %3d%%", st->batt);
    lv_obj_t * bl = lv_label_create(scr);
    /* battery bar follows state color (accent default; red when low) */
    uint32_t bar_color = (st->batt < 20) ? 0xFF453A : style->accent;
    lv_obj_set_style_text_color(bl, lv_color_hex(bar_color), 0);
    lv_label_set_text(bl, bar);
    lv_obj_align(bl, LV_ALIGN_BOTTOM_LEFT, 8, -16);

    /* === bottom frame === */
    lv_obj_t * bot_bar = lv_label_create(scr);
    lv_obj_set_style_text_color(bot_bar, lv_color_hex(style->fg_secondary), 0);
    lv_label_set_text(bot_bar, "+--------------------------------------+");
    lv_obj_align(bot_bar, LV_ALIGN_BOTTOM_LEFT, 4, -2);
}
