/*!
    \file    desktop_logic.h
    \brief   "硬装" — desktop 控制层：状态、主循环、theme 轮播

    本层不知道 theme 怎么画，只负责：
    - 持有 desktop_state_t（fake clock / steps / hr / battery / uptime）
    - 每 50ms tick：推 lv_timer_handler + hw_delay
    - 每 ~3s 加速时钟 +1 分钟、累积步数等
    - 每 5s 切下一个 theme，调对应 render fn

    "软装"由 theme_descriptor_t 注入：name + render fn 指针。
*/
#ifndef DESKTOP_LOGIC_H
#define DESKTOP_LOGIC_H

#include <stdint.h>
#include "lvgl.h"
#include "themes/theme_style.h"

typedef struct {
    int hh, mm;
    int steps;
    int hr;
    int batt;
    uint32_t uptime_s;
} desktop_state_t;

typedef void (*theme_render_fn_t)(lv_obj_t * scr,
                                  const desktop_state_t * state,
                                  const theme_style_t   * style);

typedef struct {
    const char *          name;
    theme_render_fn_t     render;
    const theme_style_t * style; /* 软装：注入 */
} theme_descriptor_t;

/* never returns — owns the main loop */
void desktop_run(const theme_descriptor_t * themes, int n_themes);

#endif /* DESKTOP_LOGIC_H */
