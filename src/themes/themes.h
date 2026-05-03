/*!
    \file    themes/themes.h
    \brief   3 个 theme renderer + font pool 共享给软装层
*/
#ifndef THEMES_H
#define THEMES_H

#include "../desktop_logic.h"
#include "theme_style.h"

/* font pool — loaded once in main.c from SD; index by px size */
typedef struct {
    lv_font_t * f14;
    lv_font_t * f18;
    lv_font_t * f24;
    lv_font_t * f28;
    lv_font_t * f32;
} font_pool_t;

extern font_pool_t g_fonts;

/* helper: resolve a px size to a font*; falls back to f28 if size unsupported */
const lv_font_t * font_pick(int px_size);

void theme_terminal_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);
void theme_pixel_render   (lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);
void theme_zen_render     (lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);

#endif
