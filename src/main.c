/*!
    \file    main.c
    \brief   v0.2 sk: 3-theme cool desktop, auto-switch every 5s, all assets from SD
*/

#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"
#include "sdcard.h"

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "src/font/binfont_loader/lv_binfont_loader.h"

extern void lv_fs_fatfs_init(void);
extern const uint8_t  montserrat_28_bin_data[];
extern const uint32_t montserrat_28_bin_size;

static FATFS s_fs;
static char  s_path[4];

/* shared big font loaded once from SD */
static lv_font_t * font_big = NULL;

/* fake desktop state — pretend it's a smartwatch */
typedef struct {
    int hh, mm;
    int steps;
    int hr;
    int batt;
    uint32_t uptime_s;
} desktop_state_t;

static desktop_state_t g_state = {
    .hh = 14, .mm = 27, .steps = 8423, .hr = 72, .batt = 87, .uptime_s = 0,
};

static sd_error_enum sd_full_init(sd_card_info_struct * info)
{
    sd_error_enum err = sd_init();
    if (err == SD_OK) err = sd_card_information_get(info);
    if (err == SD_OK) err = sd_card_select_deselect(info->card_rca);
    if (err == SD_OK) err = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
    if (err == SD_OK) err = sd_transfer_mode_config(SD_POLLING_MODE);
    return err;
}

static void install_font_if_missing(void)
{
    FIL fp;
    if (f_open(&fp, "0:/montserrat_28.bin", FA_READ) == FR_OK) { f_close(&fp); return; }
    if (f_open(&fp, "0:/montserrat_28.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    UINT bw = 0;
    f_write(&fp, montserrat_28_bin_data, montserrat_28_bin_size, &bw);
    f_close(&fp);
}

/* ---------- helpers (single-label chip — 1 widget per stat) ---------- */
static void chip_single(lv_obj_t * parent, int x, int y,
                        lv_color_t accent, const char * text)
{
    lv_obj_t * lab = lv_label_create(parent);
    lv_obj_set_pos(lab, x, y);
    lv_obj_set_style_text_color(lab, accent, 0);
    lv_label_set_text(lab, text);
}

/* ---------- THEME A: TERMINAL — mono green, ASCII art (≤6 widgets) ---------- */
static void theme_cyberpunk(lv_obj_t * scr, const desktop_state_t * st)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    /* prompt-style header */
    lv_obj_t * prompt = lv_label_create(scr);
    lv_obj_set_style_text_color(prompt, lv_color_hex(0x00FF41), 0);
    lv_label_set_text(prompt, "[user@watch ~]$ uptime");
    lv_obj_align(prompt, LV_ALIGN_TOP_LEFT, 8, 8);

    /* center clock — terminal green, big font */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(0x00FF41), 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_CENTER, 0, -10);

    /* ASCII bar bg of battery within a single label */
    int bar_filled = st->batt / 10; /* 0..10 */
    char bar[40] = "BAT [";
    int p = 5;
    for (int i = 0; i < 10; i++) bar[p++] = (i < bar_filled) ? '#' : '.';
    sprintf(bar + p, "] %d%%", st->batt);
    lv_obj_t * bl = lv_label_create(scr);
    lv_obj_set_style_text_color(bl, lv_color_hex(0x00FF41), 0);
    lv_label_set_text(bl, bar);
    lv_obj_align(bl, LV_ALIGN_BOTTOM_LEFT, 8, -22);

    /* stats row */
    char stat[40];
    sprintf(stat, "hr=%d  steps=%d  up=%lus",
            st->hr, st->steps, (unsigned long)st->uptime_s);
    lv_obj_t * sl = lv_label_create(scr);
    lv_obj_set_style_text_color(sl, lv_color_hex(0x00B82E), 0);
    lv_label_set_text(sl, stat);
    lv_obj_align(sl, LV_ALIGN_BOTTOM_LEFT, 8, -6);
}

/* ---------- THEME B: PIXEL — single accent blue, restrained (≤7 widgets) ---------- */
static void theme_modular(lv_obj_t * scr, const desktop_state_t * st)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101418), 0); /* not pure black, softer */

    /* top date — gray, label only */
    lv_obj_t * day = lv_label_create(scr);
    lv_obj_set_style_text_color(day, lv_color_hex(0x9AA0A6), 0);
    lv_label_set_text(day, "FRI  MAY 2");
    lv_obj_align(day, LV_ALIGN_TOP_LEFT, 16, 14);

    /* top-right battery (mono accent blue) */
    char bb[16];
    sprintf(bb, LV_SYMBOL_BATTERY_FULL "  %d%%", st->batt);
    lv_obj_t * bat = lv_label_create(scr);
    lv_obj_set_style_text_color(bat, lv_color_hex(0x4FC3F7), 0);
    lv_label_set_text(bat, bb);
    lv_obj_align(bat, LV_ALIGN_TOP_RIGHT, -16, 14);

    /* main clock — left-aligned big white */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(0xE8EAED), 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_LEFT_MID, 16, -4);

    /* tiny accent dot under time */
    lv_obj_t * dot = lv_obj_create(scr);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_pos(dot, 22, 134);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0x4FC3F7), 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dot, 3, 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

    /* right column: 2 mono lines, gray + accent */
    char l1[24], l2[24];
    sprintf(l1, "%d steps", st->steps);
    sprintf(l2, "%d bpm",   st->hr);

    lv_obj_t * s1 = lv_label_create(scr);
    lv_obj_set_style_text_color(s1, lv_color_hex(0xE8EAED), 0);
    lv_label_set_text(s1, l1);
    lv_obj_align(s1, LV_ALIGN_RIGHT_MID, -16, -8);

    lv_obj_t * s2 = lv_label_create(scr);
    lv_obj_set_style_text_color(s2, lv_color_hex(0x9AA0A6), 0);
    lv_label_set_text(s2, l2);
    lv_obj_align(s2, LV_ALIGN_RIGHT_MID, -16, 14);
}

/* ---------- THEME C: ZEN — battery half-arc anchor (≤7 widgets) ---------- */
static void theme_minimal(lv_obj_t * scr, const desktop_state_t * st)
{
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    /* tiny weekday at top */
    lv_obj_t * day = lv_label_create(scr);
    lv_obj_set_style_text_color(day, lv_color_hex(0x707070), 0);
    lv_label_set_text(day, "tuesday");
    lv_obj_align(day, LV_ALIGN_TOP_MID, 0, 12);

    /* main clock — large white centered */
    char tbuf[8];
    sprintf(tbuf, "%02d:%02d", st->hh, st->mm);
    lv_obj_t * tlab = lv_label_create(scr);
    if (font_big) lv_obj_set_style_text_font(tlab, font_big, 0);
    lv_obj_set_style_text_color(tlab, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(tlab, tbuf);
    lv_obj_align(tlab, LV_ALIGN_CENTER, 0, -8);

    /* date line tiny under clock */
    lv_obj_t * sub = lv_label_create(scr);
    lv_obj_set_style_text_color(sub, lv_color_hex(0x9E7B4A), 0);
    lv_label_set_text(sub, "may 2  ·  2026");
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, 22);

    /* battery half-arc anchor at bottom */
    lv_obj_t * arc = lv_arc_create(scr);
    lv_obj_set_size(arc, 120, 60);
    lv_arc_set_bg_angles(arc, 180, 360);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, st->batt);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xD4A373), LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, 26);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    char b[16];
    sprintf(b, "%d%%", st->batt);
    lv_obj_t * bl = lv_label_create(scr);
    lv_obj_set_style_text_color(bl, lv_color_hex(0xD4A373), 0);
    lv_label_set_text(bl, b);
    lv_obj_align(bl, LV_ALIGN_BOTTOM_MID, 0, -2);
}

typedef void (*theme_fn_t)(lv_obj_t *, const desktop_state_t *);
static theme_fn_t themes[3] = { theme_cyberpunk, theme_modular, theme_minimal };
static const char * theme_names[3] = { "CYBERPUNK", "MODULAR", "MINIMAL" };

int main(void)
{
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);

    POINT_COLOR = WHITE; BACK_COLOR = BLACK;
    LCD_ShowString(0, 0, 320, 16, 12, "v0.2 sk: cool desktop");

    /* SD + FATFS bring-up */
    sd_card_info_struct info;
    if (sd_full_init(&info) != SD_OK) {
        POINT_COLOR = RED;
        LCD_ShowString(0, 16, 320, 16, 12, "SD init FAIL");
        while (1) { delay(500); }
    }
    if (FATFS_LinkDriver(&sd_diskio_drv, s_path) != 0
        || f_mount(&s_fs, s_path, 1) != FR_OK) {
        POINT_COLOR = RED;
        LCD_ShowString(0, 16, 320, 16, 12, "FATFS mount FAIL");
        while (1) { delay(500); }
    }
    install_font_if_missing();

    lv_init();
    lv_port_disp_init();
    lv_fs_fatfs_init();

    font_big = lv_binfont_create("S:/montserrat_28.bin");
    lv_obj_t * scr = lv_screen_active();

    /* main loop: rotate theme every 5 seconds */
    int theme_idx = 0;
    uint32_t tick_count = 0;          /* delay(50) ticks */
    uint32_t last_minute_tick = 0;
    uint32_t last_theme_tick = 0;

    /* initial draw */
    lv_obj_clean(scr);
    themes[theme_idx](scr, &g_state);

    while (1) {
        tick_count++;

        /* fake clock advance: +1 minute every 60 ticks (60 * 50ms = 3s; "fast clock") */
        if (tick_count - last_minute_tick >= 60) {
            last_minute_tick = tick_count;
            g_state.mm++;
            if (g_state.mm >= 60) { g_state.mm = 0; g_state.hh = (g_state.hh + 1) % 24; }
            g_state.uptime_s = tick_count / 20;     /* 50ms ticks → seconds */
            g_state.steps += 7;
            g_state.hr = 65 + (tick_count % 30);
            /* redraw current theme so values update */
            lv_obj_clean(scr);
            themes[theme_idx](scr, &g_state);
        }

        /* swap theme every 100 ticks (≈5s) */
        if (tick_count - last_theme_tick >= 100) {
            last_theme_tick = tick_count;
            theme_idx = (theme_idx + 1) % 3;
            lv_obj_clean(scr);
            themes[theme_idx](scr, &g_state);
            (void)theme_names; /* could be used for HUD overlay */
        }

        lv_timer_handler();
        delay(50);
    }
}
