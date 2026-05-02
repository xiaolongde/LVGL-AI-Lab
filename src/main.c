/*!
    \file    main.c
    \brief   v0.2 sh-fatfs-port: SD bring-up + FatFs mount + read README.TXT
*/

#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"
#include "sdcard.h"

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

static FATFS s_fs;
static FIL   s_fp;
static char  s_path[4]; /* "0:/" */

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

    POINT_COLOR = WHITE;
    BACK_COLOR  = BLACK;
    LCD_ShowString(0, 0, 320, 16, 16, "v0.2 sh-fatfs-port");
    LCD_ShowString(0, 20, 320, 16, 12, "SD + FatFs R0.13c");

    char buf[80];

    /* === step 1: SD full init pipeline (matches KIT example sd_io_init) === */
    LCD_ShowString(0, 40, 320, 16, 12, "1) sd_io_init pipeline...");
    sd_error_enum err = sd_init();
    sd_card_info_struct info;
    if (err == SD_OK) err = sd_card_information_get(&info);
    if (err == SD_OK) err = sd_card_select_deselect(info.card_rca);   /* CMD7 → transfer */
    if (err == SD_OK) err = sd_bus_mode_config(SDIO_BUSMODE_4BIT);    /* 4-bit bus */
    if (err == SD_OK) err = sd_transfer_mode_config(SD_POLLING_MODE); /* polling */
    if (err != SD_OK) {
        POINT_COLOR = RED;
        sprintf(buf, "SD FAIL: %s", sd_err_str(err));
        LCD_ShowString(0, 55, 320, 16, 12, buf);
        while (1) { delay(500); }
    }
    POINT_COLOR = GREEN;
    LCD_ShowString(0, 55, 320, 16, 12, "   SD OK (transfer+4bit)");

    uint32_t cap_kb = sd_card_capacity_get();
    POINT_COLOR = WHITE;
    sprintf(buf, "   cap=%lu MB", (unsigned long)(cap_kb / 1024u));
    LCD_ShowString(0, 70, 320, 16, 12, buf);

    /* === step 1.5: raw sector 0 read (isolate FatFs glue vs SDIO data path) === */
    LCD_ShowString(0, 90, 320, 16, 12, "1.5) raw sd_block_read s0...");
    static uint32_t raw[128] __attribute__((aligned(4))); /* 512B */
    sd_error_enum rerr = sd_block_read(raw, 0, 512);
    POINT_COLOR = (rerr == SD_OK) ? GREEN : RED;
    sprintf(buf, "   rd=%s sig=%02X %02X (want 55 AA)",
            sd_err_str(rerr),
            ((uint8_t *)raw)[510], ((uint8_t *)raw)[511]);
    LCD_ShowString(0, 105, 320, 16, 12, buf);
    POINT_COLOR = WHITE;
    sprintf(buf, "   first8: %02X%02X%02X%02X %02X%02X%02X%02X",
            ((uint8_t *)raw)[0], ((uint8_t *)raw)[1], ((uint8_t *)raw)[2], ((uint8_t *)raw)[3],
            ((uint8_t *)raw)[4], ((uint8_t *)raw)[5], ((uint8_t *)raw)[6], ((uint8_t *)raw)[7]);
    LCD_ShowString(0, 120, 320, 16, 12, buf);

    /* === step 2: FATFS_LinkDriver === */
    LCD_ShowString(0, 138, 320, 16, 12, "2) link...");
    uint8_t link_res = FATFS_LinkDriver(&sd_diskio_drv, s_path);
    POINT_COLOR = (link_res == 0) ? GREEN : RED;
    sprintf(buf, "   link=%u path=\"%s\"", link_res, s_path);
    LCD_ShowString(0, 153, 320, 16, 12, buf);

    /* === step 3: f_mount === */
    POINT_COLOR = WHITE;
    LCD_ShowString(0, 168, 320, 16, 12, "3) f_mount...");
    FRESULT fr = f_mount(&s_fs, s_path, 1); /* 1 = mount immediately */
    POINT_COLOR = (fr == FR_OK) ? GREEN : RED;
    sprintf(buf, "   mount=%d (FR_OK=0)", (int)fr);
    LCD_ShowString(0, 183, 320, 16, 12, buf);

    if (fr == FR_OK) {
        /* === step 4: f_open + f_read README.TXT === */
        POINT_COLOR = WHITE;
        fr = f_open(&s_fp, "0:/README.TXT", FA_READ);
        char content[40];
        UINT br = 0;
        if (fr == FR_OK) {
            fr = f_read(&s_fp, content, sizeof(content) - 1, &br);
            content[br] = 0;
            f_close(&s_fp);
            for (UINT i = 0; i < br; i++) {
                if (content[i] < 0x20 || content[i] > 0x7E) content[i] = '.';
            }
        } else {
            content[0] = 0;
        }
        POINT_COLOR = (fr == FR_OK) ? GREEN : RED;
        sprintf(buf, "4) open=%d br=%u", (int)fr, (unsigned)br);
        LCD_ShowString(0, 200, 320, 16, 12, buf);
        if (br > 0) {
            POINT_COLOR = YELLOW;
            LCD_ShowString(0, 215, 320, 16, 12, content);
        }
    }

    /* heartbeat blink */
    int blink = 0;
    while (1) {
        blink ^= 1;
        LCD_Fill(310, 0, 319, 4, blink ? GREEN : BLACK);
        delay(500);
    }
}
