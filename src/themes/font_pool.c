/*!
    \file    themes/font_pool.c
    \brief   font_pool_t global + font_pick resolver
*/

#include "themes.h"

font_pool_t g_fonts = { NULL, NULL, NULL, NULL, NULL };

const lv_font_t * font_pick(int px)
{
    switch (px) {
        case 14: return g_fonts.f14 ? g_fonts.f14 : g_fonts.f28;
        case 18: return g_fonts.f18 ? g_fonts.f18 : g_fonts.f28;
        case 24: return g_fonts.f24 ? g_fonts.f24 : g_fonts.f28;
        case 32: return g_fonts.f32 ? g_fonts.f32 : g_fonts.f28;
        case 28:
        default: return g_fonts.f28;
    }
}
