/*!
    \file    themes/style_loader.c
*/

#include "style_loader.h"
#include "lvgl.h"
#include <string.h>

#define STYLE_BUF_MAX 1024

int style_load(theme_style_t * out, const char * path, const char * default_text)
{
    /* always seed with defaults first (so unknown keys keep defaults) */
    memset(out, 0, sizeof(*out));
    if (default_text) {
        theme_style_parse(out, default_text, strlen(default_text));
    }

    lv_fs_file_t fp;
    if (lv_fs_open(&fp, path, LV_FS_MODE_RD) != LV_FS_RES_OK) {
        return 0; /* SD miss → keep defaults */
    }

    static char buf[STYLE_BUF_MAX]; /* shared scratch — only one load at a time */
    uint32_t br = 0;
    lv_fs_read(&fp, buf, sizeof(buf) - 1, &br);
    lv_fs_close(&fp);
    buf[br] = 0;

    /* SD-loaded values override defaults */
    theme_style_parse(out, buf, br);
    return 1;
}
