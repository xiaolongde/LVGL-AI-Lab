/*!
    \file    themes/theme_style.h
    \brief   软装 descriptor — 颜色/字符串 token，文本格式从 SD/内存解析

    .tstyle 文件格式（key=value 一行一条；# 开头注释）：
    ----------------------------------------------------------------
    # terminal.tstyle
    bg          = 0x000000
    accent      = 0x00FF41
    fg_main     = 0x00FF41
    fg_secondary= 0x00B82E
    fg_muted    = 0x404040
    banner      = [user@watch ~]$ uptime
    sub_text    = may 2  ·  2026
    ----------------------------------------------------------------
*/
#ifndef THEME_STYLE_H
#define THEME_STYLE_H

#include <stdint.h>
#include <stddef.h>

#define THEME_STR_MAX 40

typedef struct {
    /* 5 color tokens — RGB888 */
    uint32_t bg;
    uint32_t accent;
    uint32_t fg_main;
    uint32_t fg_secondary;
    uint32_t fg_muted;

    /* string tokens */
    char banner[THEME_STR_MAX];   /* theme-specific banner / prompt */
    char sub_text[THEME_STR_MAX]; /* secondary line (date / weekday) */

    /* font sizes (px) — resolved to lv_font_t* via font_pool at render time */
    int font_main;       /* primary font: clock */
    int font_secondary;  /* secondary font: sub-text / stat lines */
} theme_style_t;

/* parse a textual descriptor (in `text`, length `len`) into `out`.
   Unknown keys are ignored. Returns 0 on success (always; lenient). */
int theme_style_parse(theme_style_t * out, const char * text, size_t len);

#endif /* THEME_STYLE_H */
