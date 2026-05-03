/*!
    \file    themes/themes.h
    \brief   3 个 theme renderer — 软装通过 theme_style_t 注入
*/
#ifndef THEMES_H
#define THEMES_H

#include "../desktop_logic.h"
#include "theme_style.h"

extern lv_font_t * font_big;

/* render API: 同一 widget 树骨架，软装由 style 决定 */
void theme_terminal_render(lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);
void theme_pixel_render   (lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);
void theme_zen_render     (lv_obj_t * scr, const desktop_state_t * st, const theme_style_t * style);

#endif
