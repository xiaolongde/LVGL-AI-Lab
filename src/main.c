/*!
    \file    main.c
    \brief   LVGL-AI-Lab — Card 2 sb-lvgl-port demo
             LVGL v9.5.0 on ST7789V3 1.47" (320x172) via SPI2
             静态 banner + 动态 tick 计数器（证明 LVGL timer + flush 在持续工作）
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

static lv_obj_t * s_time_label;
static lv_obj_t * s_progress_bar;

/* lv_timer 回调：每秒读 RTC 更新时间 label + 进度条秒数 */
static void second_timer_cb(lv_timer_t * t)
{
    (void)t;
    uint8_t h, m, s;
    rtc_get_time(&h, &m, &s);

    char buf[16];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    lv_label_set_text(s_time_label, buf);

    /* 进度条显示当前秒（0-59 映射 0-100） */
    lv_bar_set_value(s_progress_bar, (int32_t)((uint32_t)s * 100u / 59u), LV_ANIM_OFF);
}

int main(void)
{
    /* 1. HW bring-up */
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);
    rtc_user_init();        /* LSE + RTC 1Hz counter, 默认 12:00:00 */

    /* 2. LVGL init */
    lv_init();
    lv_port_disp_init();

    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    /* (banner 已移除——sysmon overlay 占了顶部 + 实质内容是时间 + spinner) */

    /* 4. 时间 label 移到屏左侧，把屏中央让给大 spinner 测全屏 FPS */
    s_time_label = lv_label_create(scr);
    lv_label_set_text(s_time_label, "12:00:00");
    lv_obj_set_style_text_color(s_time_label, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_align(s_time_label, LV_ALIGN_LEFT_MID, 4, 0);

    /* 5. 秒进度条 */
    s_progress_bar = lv_bar_create(scr);
    lv_obj_set_size(s_progress_bar, 280, 12);
    lv_obj_align(s_progress_bar, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_bar_set_range(s_progress_bar, 0, 100);
    lv_bar_set_value(s_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_make(0x33, 0x33, 0x33), 0);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_make(0x00, 0xAA, 0xFF), LV_PART_INDICATOR);

    /* 6. 1 秒 timer 读 RTC */
    lv_timer_create(second_timer_cb, 1000, NULL);

    /* 7. 大 spinner 占据屏右半中央 ~120x120，每帧 invalidate 大区域，逼近全屏 FPS 上限 */
    lv_obj_t * spinner = lv_spinner_create(scr);
    lv_obj_set_size(spinner, 120, 120);
    lv_obj_align(spinner, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_arc_color(spinner, lv_color_make(0xFF, 0x00, 0xFF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_make(0x33, 0x33, 0x33), LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_MAIN);

    /* 8. Main loop —— delay 1ms 追求最高 FPS */
    while (1) {
        lv_timer_handler();
        delay(1);
    }
}
