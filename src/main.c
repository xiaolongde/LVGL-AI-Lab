/*!
    \file    main.c
    \brief   v0.3 + 多 size 字体 SD 加载（5 fonts in font_pool）
*/

#include "hw.h"
#include "lvgl.h"
#include "src/font/binfont_loader/lv_binfont_loader.h"

#include "desktop_logic.h"
#include "themes/themes.h"
#include "themes/style_loader.h"
#include "snapshot_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* embedded defaults — written to SD on first boot if missing */
static const char DEFAULT_TERMINAL_TSTYLE[] =
    "bg             = 0x000000\n"
    "accent         = 0x00FF41\n"
    "fg_main        = 0x00FF41\n"
    "fg_secondary   = 0x00B82E\n"
    "fg_muted       = 0x404040\n"
    "banner         = [user@watch ~]$ uptime\n"
    "sub_text       = (unused on terminal)\n"
    "font_main      = 28\n"
    "font_secondary = 14\n";

static const char DEFAULT_PIXEL_TSTYLE[] =
    "bg             = 0x101418\n"
    "accent         = 0x4FC3F7\n"
    "fg_main        = 0xE8EAED\n"
    "fg_secondary   = 0x9AA0A6\n"
    "fg_muted       = 0x707070\n"
    "banner         = (unused on pixel)\n"
    "sub_text       = FRI  MAY 2\n"
    "font_main      = 28\n"
    "font_secondary = 18\n";

static const char DEFAULT_ZEN_TSTYLE[] =
    "bg             = 0x000000\n"
    "accent         = 0xD4A373\n"
    "fg_main        = 0xFFFFFF\n"
    "fg_secondary   = 0x202020\n"
    "fg_muted       = 0x707070\n"
    "banner         = tuesday\n"
    "sub_text       = may 2  \xC2\xB7  2026\n"
    "font_main      = 32\n"
    "font_secondary = 18\n";

static void install_style_if_missing(const char * path, const char * content)
{
    lv_fs_file_t fp;
    if (lv_fs_open(&fp, path, LV_FS_MODE_RD) == LV_FS_RES_OK) {
        lv_fs_close(&fp);
        return;
    }
    if (lv_fs_open(&fp, path, LV_FS_MODE_WR) != LV_FS_RES_OK) return;
    uint32_t bw = 0;
    lv_fs_write(&fp, content, (uint32_t)strlen(content), &bw);
    lv_fs_close(&fp);
}

static theme_style_t s_term_style;
static theme_style_t s_pixel_style;
static theme_style_t s_zen_style;

#ifdef BUILD_HOST_SIM
int main(int argc, char ** argv)
#else
int main(void)
#endif
{
    int boot_rc = hw_boot();
    if (boot_rc != 0) {
        char b[40];
        sprintf(b, "hw_boot FAIL rc=%d", boot_rc);
        hw_boot_log(16, b);
        while (1) hw_delay(500);
    }

    hw_install_font();

    lv_init();
    hw_lv_disp_init();
    hw_lv_fs_init();

    g_fonts.f14 = lv_binfont_create("S:/montserrat_14.bin");
    g_fonts.f18 = lv_binfont_create("S:/montserrat_18.bin");
    g_fonts.f24 = lv_binfont_create("S:/montserrat_24.bin");
    g_fonts.f28 = lv_binfont_create("S:/montserrat_28.bin");
    g_fonts.f32 = lv_binfont_create("S:/montserrat_32.bin");

    install_style_if_missing("S:/terminal.tstyle", DEFAULT_TERMINAL_TSTYLE);
    install_style_if_missing("S:/pixel.tstyle",    DEFAULT_PIXEL_TSTYLE);
    install_style_if_missing("S:/zen.tstyle",      DEFAULT_ZEN_TSTYLE);

    style_load(&s_term_style,  "S:/terminal.tstyle", DEFAULT_TERMINAL_TSTYLE);
    style_load(&s_pixel_style, "S:/pixel.tstyle",    DEFAULT_PIXEL_TSTYLE);
    style_load(&s_zen_style,   "S:/zen.tstyle",      DEFAULT_ZEN_TSTYLE);

    static const theme_descriptor_t themes[3] = {
        { "TERMINAL", theme_terminal_render, &s_term_style  },
        { "PIXEL",    theme_pixel_render,    &s_pixel_style },
        { "ZEN",      theme_zen_render,      &s_zen_style   },
    };

#ifdef BUILD_HOST_SIM
    /* --snapshot <dir>: 渲染每个 theme 一次 → 抓 PPM → 退出 */
    if (argc >= 3 && strcmp(argv[1], "--snapshot") == 0) {
        const char * dir = argv[2];
        char log_path[256];
        snprintf(log_path, sizeof(log_path), "%s/_snapshot.log", dir);
        FILE * lf = fopen(log_path, "w");
        if (lf) fprintf(lf, "snapshot mode start, dir=%s\n", dir);
        /* fixed fake state for reproducible snapshots */
        desktop_state_t s = { .hh = 12, .mm = 34, .steps = 8423,
                              .hr = 72, .batt = 87, .uptime_s = 42 };
        lv_obj_t * scr = lv_screen_active();
        /* let LVGL display flush at least once before snapshot */
        for (int warmup = 0; warmup < 3; warmup++) {
            lv_timer_handler();
            hw_delay(20);
        }
        for (int i = 0; i < 3; i++) {
            lv_obj_clean(scr);
            themes[i].render(scr, &s, themes[i].style);
            for (int k = 0; k < 8; k++) { lv_timer_handler(); hw_delay(30); }
            lv_obj_update_layout(scr);
            int32_t w = lv_obj_get_width(scr);
            int32_t h = lv_obj_get_height(scr);
            if (lf) { fprintf(lf, "[%d] scr w=%ld h=%ld\n", i, (long)w, (long)h); fflush(lf); }
            char path[256];
            snprintf(path, sizeof(path), "%s/%s.ppm", dir, themes[i].name);
            int rc = snapshot_screen_to_ppm(path);
            if (lf) { fprintf(lf, "[snapshot] %s -> rc=%d\n", path, rc); fflush(lf); }
        }
        if (lf) { fprintf(lf, "snapshot mode done\n"); fclose(lf); }
        return 0;
    }
#endif

    desktop_run(themes, 3);
    return 0;
}
