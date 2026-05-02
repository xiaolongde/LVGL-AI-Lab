/*!
    \file    main.c
    \brief   v0.2 sg-sdio-bringup test：SD 卡硬件层验证（无 LVGL，KIT TFTLCD 直接显示 status）
*/

#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"
#include "sdcard.h"

static const char * sd_type_str(uint8_t t)
{
    switch (t) {
        case SDIO_STD_CAPACITY_SD_CARD_V1_1:  return "SDv1.1";
        case SDIO_STD_CAPACITY_SD_CARD_V2_0:  return "SDv2.0";
        case SDIO_HIGH_CAPACITY_SD_CARD:      return "SDHC";
        case SDIO_MULTIMEDIA_CARD:            return "MMC";
        case SDIO_SECURE_DIGITAL_IO_CARD:     return "SDIO";
        default:                              return "UNK";
    }
}

static const char * sd_err_str(sd_error_enum e)
{
    switch (e) {
        case SD_OK: return "OK";
        case SD_CMD_RESP_TIMEOUT: return "CMD_TIMEOUT";
        case SD_DATA_TIMEOUT: return "DATA_TIMEOUT";
        case SD_ERROR: return "GENERIC_ERR";
        default: return "ERR";
    }
}

int main(void)
{
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);

    /* 简单 banner */
    POINT_COLOR = WHITE;
    BACK_COLOR  = BLACK;
    LCD_ShowString(0, 0, 320, 16, 16, "v0.2 sg-sdio-bringup");
    LCD_ShowString(0, 20, 320, 16, 12, "GD32F303 + microSD via SDIO");

    /* SD init */
    LCD_ShowString(0, 50, 320, 16, 16, "SD init...");
    sd_error_enum err = sd_init();

    if (err != SD_OK) {
        char buf[64];
        sprintf(buf, "FAIL: %s (code %d)", sd_err_str(err), (int)err);
        POINT_COLOR = RED;
        LCD_ShowString(0, 70, 320, 16, 16, buf);
        LCD_ShowString(0, 100, 320, 16, 12, "Check: SD inserted? FAT32?");
        while (1) { delay(500); }
    }

    /* Get card info */
    sd_card_info_struct info;
    err = sd_card_information_get(&info);
    if (err != SD_OK) {
        POINT_COLOR = RED;
        LCD_ShowString(0, 70, 320, 16, 16, "info_get FAIL");
        while (1) { delay(500); }
    }

    /* Display info */
    POINT_COLOR = GREEN;
    LCD_ShowString(0, 70, 320, 16, 16, "SD OK!");

    char buf[64];
    POINT_COLOR = WHITE;
    sprintf(buf, "Type: %s", sd_type_str(info.card_type));
    LCD_ShowString(0, 95, 320, 16, 12, buf);

    /* Capacity in MB: capacity field is u64 in bytes; sd_card_capacity_get returns KB */
    uint32_t cap_kb = sd_card_capacity_get();
    sprintf(buf, "Capacity: %lu MB", (unsigned long)(cap_kb / 1024u));
    LCD_ShowString(0, 110, 320, 16, 12, buf);

    sprintf(buf, "RCA: 0x%04lX  BlkSize: %lu",
            (unsigned long)info.card_rca, (unsigned long)info.card_blocksize);
    LCD_ShowString(0, 125, 320, 16, 12, buf);

    sprintf(buf, "MID:0x%02lX  S/N:0x%08lX",
            (unsigned long)info.card_cid.mid,
            (unsigned long)info.card_cid.psn);
    LCD_ShowString(0, 140, 320, 16, 12, buf);

    /* 心跳：屏底闪个像素证明 main loop 在跑 */
    int blink = 0;
    while (1) {
        blink ^= 1;
        LCD_Fill(0, 165, 5, 170, blink ? GREEN : BLACK);
        delay(500);
    }
}
