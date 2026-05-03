/*!
    \file    desktop_logic.c
    \brief   硬装 — desktop 主循环 + state advance + theme 轮播
*/

#include "desktop_logic.h"
#include "hw.h"

static desktop_state_t g_state = {
    .hh = 14, .mm = 27, .steps = 8423, .hr = 72, .batt = 87, .uptime_s = 0,
};

void desktop_run(const theme_descriptor_t * themes, int n_themes)
{
    if (themes == NULL || n_themes <= 0) return;

    lv_obj_t * scr = lv_screen_active();
    int      theme_idx = 0;
    uint32_t tick_count = 0;
    uint32_t last_minute_tick = 0;
    uint32_t last_theme_tick = 0;

    lv_obj_clean(scr);
    themes[theme_idx].render(scr, &g_state, themes[theme_idx].style);

    while (1) {
        tick_count++;

        /* 1s wall = state tick + redraw (drives cursor blink, batt sweep, hr, etc) */
        if (tick_count - last_minute_tick >= 20) {
            last_minute_tick = tick_count;
            g_state.mm++;
            if (g_state.mm >= 60) {
                g_state.mm = 0;
                g_state.hh = (g_state.hh + 1) % 24;
            }
            g_state.uptime_s = tick_count / 20;
            g_state.steps += 7;
            g_state.hr = 65 + (tick_count % 30);
            /* fake batt sweep 100→0→100 to expose state-color thresholds */
            {
                static int batt_dir = -1;
                g_state.batt += batt_dir * 5;
                if (g_state.batt <= 0)   { g_state.batt = 0;   batt_dir =  1; }
                if (g_state.batt >= 100) { g_state.batt = 100; batt_dir = -1; }
            }
            lv_obj_clean(scr);
            themes[theme_idx].render(scr, &g_state, themes[theme_idx].style);
        }

        /* ~5s wall = next theme */
        if (tick_count - last_theme_tick >= 100) {
            last_theme_tick = tick_count;
            theme_idx = (theme_idx + 1) % n_themes;
            lv_obj_clean(scr);
            themes[theme_idx].render(scr, &g_state, themes[theme_idx].style);
        }

        lv_timer_handler();
        hw_delay(50);
    }
}
