/*!
    \file    themes/style_loader.h
    \brief   读 .tstyle 从 LVGL fs (S:/themes/X.tstyle)，失败则用嵌入 default
*/
#ifndef STYLE_LOADER_H
#define STYLE_LOADER_H

#include "theme_style.h"

/* 加载顺序：(1) 试 lv_fs_open(path) → 读完 + parse；
              (2) 失败则把 default_text parse 进 out。
   返回 1 = SD 命中，0 = 用 default。 */
int  style_load(theme_style_t * out, const char * path, const char * default_text);

#endif
