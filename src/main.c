/*!
    \file    main.c
    \brief   v0.3 m3: 软装从 S:/<theme>.tstyle 加载，缺失则用嵌入 default
             并自装到 SD（hands-off）
*/

#include "hw.h"
#include "lvgl.h"
#include "src/font/binfont_loader/lv_binfont_loader.h"

#include "desktop_logic.h"
#include "themes/themes.h"
#include "themes/style_loader.h"

#include <stdio.h>
#include <string.h>

lv_font_t * font_big = NULL;

/* embedded defaults — one string per theme, identical to tools/styles/*.tstyle */
static const char DEFAULT_TERMINAL_TSTYLE[] =
    "bg           = 0x000000\n"
    "accent       = 0x00FF41\n"
    "fg_main      = 0x00FF41\n"
    "fg_secondary = 0x00B82E\n"
    "fg_muted     = 0x404040\n"
    "banner       = [user@watch ~]$ uptime\n"
    "sub_text     = (unused on terminal)\n";

static const char DEFAULT_PIXEL_TSTYLE[] =
    "bg           = 0x101418\n"
    "accent       = 0x4FC3F7\n"
    "fg_main      = 0xE8EAED\n"
    "fg_secondary = 0x9AA0A6\n"
    "fg_muted     = 0x707070\n"
    "banner       = (unused on pixel)\n"
    "sub_text     = FRI  MAY 2\n";

static const char DEFAULT_ZEN_TSTYLE[] =
    "bg           = 0x000000\n"
    "accent       = 0xD4A373\n"
    "fg_main      = 0xFFFFFF\n"
    "fg_secondary = 0x202020\n"
    "fg_muted     = 0x707070\n"
    "banner       = tuesday\n"
    "sub_text     = may 2  \xC2\xB7  2026\n";

/* hands-off: write embedded default to "S:/x.tstyle" if not already there.
   works on both targets via lv_fs (MCU = lv_fs_fatfs, host = lv_fs_stdio). */
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

int main(void)
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

    font_big = lv_binfont_create(hw_font_path());

    /* hands-off install + load styles */
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
    /* the array is const but the styles inside are static globals → OK */
    desktop_run(themes, 3);

    return 0;
}
