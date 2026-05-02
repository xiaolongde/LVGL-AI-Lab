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

#include "lvgl.h"
#include "lv_port_disp.h"

static lv_obj_t * s_tick_label;
static uint32_t   s_tick_value;

/* lv_timer 回调：每 100 ms 更新 tick 数字 + 走一个色环（顶部进度条） */
static lv_obj_t * s_progress_bar;

static void tick_timer_cb(lv_timer_t * t)
{
    (void)t;
    s_tick_value++;

    char buf[32];
    snprintf(buf, sizeof(buf), "tick=%lu  %.1fs", (unsigned long)s_tick_value, s_tick_value * 0.1f);
    lv_label_set_text(s_tick_label, buf);

    /* 进度条 0..100 循环 */
    int32_t v = (int32_t)(s_tick_value % 101);
    lv_bar_set_value(s_progress_bar, v, LV_ANIM_OFF);
}

int main(void)
{
    /* 1. HW bring-up */
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Clear(BLACK);

    /* 2. LVGL init */
    lv_init();
    lv_port_disp_init();

    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    /* 3. 静态 banner（顶部） */
    lv_obj_t * banner = lv_label_create(scr);
    lv_label_set_text(banner, "LVGL v9.5.0\nGD32F303 + ST7789V3");
    lv_obj_set_style_text_color(banner, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_align(banner, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(banner, LV_ALIGN_TOP_MID, 0, 10);

    /* 4. 动态 tick 计数器（中下部） */
    s_tick_label = lv_label_create(scr);
    lv_label_set_text(s_tick_label, "tick=0  0.0s");
    lv_obj_set_style_text_color(s_tick_label, lv_color_make(0x00, 0xFF, 0x00), 0);  /* 绿 */
    lv_obj_align(s_tick_label, LV_ALIGN_CENTER, 0, 20);

    /* 5. 进度条（屏底，循环 0..100） */
    s_progress_bar = lv_bar_create(scr);
    lv_obj_set_size(s_progress_bar, 280, 12);
    lv_obj_align(s_progress_bar, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_bar_set_range(s_progress_bar, 0, 100);
    lv_bar_set_value(s_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_make(0x33, 0x33, 0x33), 0);
    lv_obj_set_style_bg_color(s_progress_bar, lv_color_make(0x00, 0xAA, 0xFF), LV_PART_INDICATOR);

    /* 6. 100 ms 周期 timer 推进 tick */
    lv_timer_create(tick_timer_cb, 100, NULL);

    /* 7. Main loop */
    while (1) {
        lv_timer_handler();
        delay(5);
    }
}
