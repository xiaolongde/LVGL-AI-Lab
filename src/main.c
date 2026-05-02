/*!
    \file    main.c
    \brief   LVGL-AI-Lab — sd-watch-face: 仿华为 GT3 表盘 (Figma-spec implementation)
             Spec: docs/designs/2026-05-02-watchface-spec-v1.md
             Hardware: ST7789V3 1.47" 320x172 SPI2 + GD32F303RC + LVGL v9.5.0
*/

#include "gd32f30x.h"
#include <stdio.h>

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"
#include "rtc.h"

#include "lvgl.h"
#include "lv_port_disp.h"

/* === Color tokens (per spec) === */
#define COL_BG     lv_color_hex(0x0A0E1A)
#define COL_TRACK  lv_color_hex(0x1F2333)
#define COL_ROSE   lv_color_hex(0xD4A373)
#define COL_GREEN  lv_color_hex(0x4AD98E)
#define COL_BLUE   lv_color_hex(0x5AC8FA)
#define COL_TXT    lv_color_hex(0xFFFFFF)
#define COL_DIM    lv_color_hex(0x8C8C9E)

static lv_obj_t * s_time_label;
static lv_obj_t * s_sec_bar;

static const char * weekday_name(uint8_t wd)
{
    static const char * t[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    return (wd < 7) ? t[wd] : "---";
}

static const char * month_abbr(uint8_t mo)
{
    static const char * t[] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
    return (mo >= 1 && mo <= 12) ? t[mo - 1] : "---";
}

static void second_timer_cb(lv_timer_t * t)
{
    (void)t;
    uint8_t h, m, s;
    rtc_get_time(&h, &m, &s);

    char buf[16];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    lv_label_set_text(s_time_label, buf);

    lv_bar_set_value(s_sec_bar, (int32_t)((uint32_t)s * 100u / 59u), LV_ANIM_OFF);
}

/* GT3 风格 ring：270° 开口向下 + 圆头端点 + 中心 value/unit 双 label */
static lv_obj_t * make_gt3_ring(lv_obj_t * parent, int32_t size,
                                lv_color_t indi_color, int32_t value,
                                const char * value_text, const char * unit_text)
{
    lv_obj_t * arc = lv_arc_create(parent);
    lv_obj_set_size(arc, size, size);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, value);
    lv_arc_set_bg_angles(arc, 135, 45);
    lv_arc_set_rotation(arc, 0);

    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_width(arc, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, COL_TRACK, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, indi_color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_MAIN);
    lv_obj_set_style_pad_all(arc, 0, 0);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);

    lv_obj_t * v = lv_label_create(arc);
    lv_label_set_text(v, value_text);
    lv_obj_set_style_text_color(v, COL_TXT, 0);
    lv_obj_align(v, LV_ALIGN_CENTER, 0, -4);

    lv_obj_t * u = lv_label_create(arc);
    lv_label_set_text(u, unit_text);
    lv_obj_set_style_text_color(u, COL_DIM, 0);
    lv_obj_align(u, LV_ALIGN_CENTER, 0, 10);

    return arc;
}

int main(void)
{
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);
    rtc_user_init();

    lv_init();
    lv_port_disp_init();

    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部日期 */
    {
        char buf[24];
        snprintf(buf, sizeof(buf), "%s  %s 02", weekday_name(6), month_abbr(5));
        lv_obj_t * date_lbl = lv_label_create(scr);
        lv_label_set_text(date_lbl, buf);
        lv_obj_set_style_text_color(date_lbl, COL_DIM, 0);
        lv_obj_set_style_text_letter_space(date_lbl, 1, 0);
        lv_obj_align(date_lbl, LV_ALIGN_TOP_MID, 0, 4);
    }

    /* 左 ring 步数 — 缩到 70x70 给中央时间留更多视觉空间 */
    lv_obj_t * step_ring = make_gt3_ring(scr, 70, COL_ROSE, 65, "6500", "STEP");
    lv_obj_align(step_ring, LV_ALIGN_LEFT_MID, 16, 0);
    (void)step_ring;

    /* 右 ring 心率 */
    lv_obj_t * hr_ring = make_gt3_ring(scr, 70, COL_GREEN, 72, "72", "BPM");
    lv_obj_align(hr_ring, LV_ALIGN_RIGHT_MID, -16, 0);
    (void)hr_ring;

    /* 中央时间 — 14px 默认字体 + 玫瑰金边框 time card 让视觉占据 hero 位
     * （transform_scale 2.0× 在 LVGL v9.5 + partial buffer 有不明 bug，4 次实验全半屏；BFS 后选 framing 路径） */
    lv_obj_t * time_card = lv_obj_create(scr);
    lv_obj_set_size(time_card, 120, 40);
    lv_obj_align(time_card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(time_card, lv_color_hex(0x141828), 0);
    lv_obj_set_style_bg_opa(time_card, LV_OPA_70, 0);
    lv_obj_set_style_border_color(time_card, COL_ROSE, 0);
    lv_obj_set_style_border_width(time_card, 1, 0);
    lv_obj_set_style_radius(time_card, 8, 0);
    lv_obj_set_style_pad_all(time_card, 0, 0);
    lv_obj_clear_flag(time_card, LV_OBJ_FLAG_SCROLLABLE);

    s_time_label = lv_label_create(time_card);
    lv_label_set_text(s_time_label, "12:00:00");
    lv_obj_set_style_text_color(s_time_label, COL_TXT, 0);
    lv_obj_set_style_text_letter_space(s_time_label, 1, 0);
    lv_obj_center(s_time_label);

    /* 底部秒条 */
    s_sec_bar = lv_bar_create(scr);
    lv_obj_set_size(s_sec_bar, 220, 3);
    lv_obj_align(s_sec_bar, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_bar_set_range(s_sec_bar, 0, 100);
    lv_bar_set_value(s_sec_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_sec_bar, COL_TRACK, 0);
    lv_obj_set_style_bg_color(s_sec_bar, COL_BLUE, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_sec_bar, 2, 0);
    lv_obj_set_style_radius(s_sec_bar, 2, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(s_sec_bar, 0, 0);

    lv_timer_create(second_timer_cb, 1000, NULL);

    while (1) {
        lv_timer_handler();
        delay(5);
    }
}
