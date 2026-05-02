/*!
    \file    main.c
    \brief   LVGL-AI-Lab — Card 2 sa-lcd-bringup demo
             SPI2 + ST7789V3 (1.14" 240x135) bring-up + LCD_ShowString sanity
*/

#include "gd32f30x.h"
#include <stdio.h>

#include "systick.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"

static char str[32];
static uint32_t tick;

int main(void)
{
    systick_config();
    gpio_config();          /* RGB LED + USER key */
    spi_lcd_init();         /* SPI2 + LCD pin GPIO + PMOS power on (PB7 low) */
    LCD_Init();             /* ST7789V3 init sequence */

    LCD_Clear(BLACK);
    BACK_COLOR = BLACK;

    /* Static info — 屏 1.47" 横屏 320x172 */
    POINT_COLOR = CYAN;
    LCD_ShowString(0,  0, 320, 16, 12, "LVGL-AI-Lab");
    POINT_COLOR = GREEN;
    LCD_ShowString(0, 14, 320, 16, 12, "ST7789V3 320x172 SPI2");
    POINT_COLOR = WHITE;
    LCD_ShowString(0, 28, 320, 16, 12, "GD32F303RC @120MHz");
    POINT_COLOR = YELLOW;
    LCD_ShowString(0, 42, 320, 16, 12, "Card2 sa-lcd-bringup");

    /* Heartbeat on screen only — LEDs stay off (gpio_config 已把它们置 HIGH = OFF) */
    POINT_COLOR = MAGENTA;
    while (1) {
        sprintf(str, "tick=%lu", (unsigned long)tick);
        LCD_ShowString(0, 60, 320, 16, 12, str);
        tick++;
        delay(500);
    }
}
