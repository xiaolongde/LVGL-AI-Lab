/*!
    \file    main.c
    \brief   v0.2 si-lvgl-fs-drv: LVGL + lv_fs_fatfs ('S:' drive over SDIO + FatFs)
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

#include "lvgl.h"
#include "lv_port_disp.h"

extern void lv_fs_fatfs_init(void); /* declared in lvgl/src/libs/fsdrv/lv_fs_fatfs.c */

static FATFS s_fs;
static char  s_path[4]; /* "0:/" */

static const char * sd_err_str(sd_error_enum e)
{
    switch (e) {
        case SD_OK: return "OK";
        case SD_CMD_RESP_TIMEOUT: return "CMD_TIMEOUT";
        case SD_DATA_TIMEOUT: return "DATA_TIMEOUT";
        default: return "ERR";
    }
}

/* full SD init pipeline (matches KIT example sd_io_init) */
static sd_error_enum sd_full_init(sd_card_info_struct * info)
{
    sd_error_enum err = sd_init();
    if (err == SD_OK) err = sd_card_information_get(info);
    if (err == SD_OK) err = sd_card_select_deselect(info->card_rca);
    if (err == SD_OK) err = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
    if (err == SD_OK) err = sd_transfer_mode_config(SD_POLLING_MODE);
    return err;
}

int main(void)
{
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);

    /* boot diagnostic banner via KIT TFTLCD (before LVGL takes over) */
    POINT_COLOR = WHITE;
    BACK_COLOR  = BLACK;
    LCD_ShowString(0, 0, 320, 16, 12, "v0.2 si: SD+FatFs+LVGL");

    char buf[80];

    /* === SD full init pipeline === */
    sd_card_info_struct info;
    sd_error_enum sderr = sd_full_init(&info);
    if (sderr != SD_OK) {
        POINT_COLOR = RED;
        sprintf(buf, "SD FAIL: %s", sd_err_str(sderr));
        LCD_ShowString(0, 16, 320, 16, 12, buf);
        while (1) { delay(500); }
    }

    /* === FATFS link + mount === */
    if (FATFS_LinkDriver(&sd_diskio_drv, s_path) != 0) {
        POINT_COLOR = RED;
        LCD_ShowString(0, 16, 320, 16, 12, "FATFS_LinkDriver fail");
        while (1) { delay(500); }
    }
    if (f_mount(&s_fs, s_path, 1) != FR_OK) {
        POINT_COLOR = RED;
        LCD_ShowString(0, 16, 320, 16, 12, "f_mount fail");
        while (1) { delay(500); }
    }

    /* === LVGL init === */
    lv_init();
    lv_port_disp_init();
    lv_fs_fatfs_init(); /* registers 'S:' drive backed by FatFs */

    /* === Build LVGL UI === */
    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0A0E1A), 0);

    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "si: lv_fs S: drive");
    lv_obj_set_style_text_color(title, lv_color_hex(0xD4A373), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 4, 4);

    /* === Pick first regular file from root, then read it via lv_fs === */
    static char picked[80] = "S:/";
    static char picked_fatfs[80] = "0:/";
    static int  found_file = 0;
    {
        DIR dir;
        FILINFO fno;
        FRESULT pr = f_opendir(&dir, "0:/");

        lv_obj_t * dir_lbl = lv_label_create(scr);
        lv_obj_set_style_text_color(dir_lbl, lv_color_hex(0xFFFF00), 0);
        lv_obj_align(dir_lbl, LV_ALIGN_TOP_LEFT, 4, 80);
        lv_obj_set_width(dir_lbl, 312);
        lv_label_set_long_mode(dir_lbl, LV_LABEL_LONG_WRAP);

        if (pr != FR_OK) {
            lv_label_set_text_fmt(dir_lbl, "opendir fail: %d", (int)pr);
        } else {
            while (1) {
                pr = f_readdir(&dir, &fno);
                if (pr != FR_OK || fno.fname[0] == 0) break;
                if ((fno.fattrib & AM_DIR) == 0 && fno.fname[0] != '.') {
                    /* found a regular file */
                    snprintf(picked + 3, sizeof(picked) - 3, "%s", fno.fname);
                    snprintf(picked_fatfs + 3, sizeof(picked_fatfs) - 3, "%s", fno.fname);
                    found_file = 1;
                    break;
                }
            }
            f_closedir(&dir);
            if (found_file) {
                lv_label_set_text_fmt(dir_lbl, "picked: %s (sz=%lu)",
                                      picked, (unsigned long)fno.fsize);
            } else {
                lv_label_set_text(dir_lbl, "no regular file in root");
            }
        }
    }

    /* === Read first-found file through LVGL fs API === */
    lv_fs_file_t f;
    lv_fs_res_t r = found_file
        ? lv_fs_open(&f, picked, LV_FS_MODE_RD)
        : LV_FS_RES_NOT_EX;

    lv_obj_t * status = lv_label_create(scr);
    lv_obj_set_style_text_color(status, lv_color_hex(0x4AD98E), 0);
    lv_obj_align(status, LV_ALIGN_TOP_LEFT, 4, 24);

    lv_obj_t * content = lv_label_create(scr);
    lv_obj_set_style_text_color(content, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 4, 50);
    lv_obj_set_width(content, 312);
    lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);

    if (r != LV_FS_RES_OK) {
        lv_label_set_text_fmt(status, "lv_fs_open: ERR %d", (int)r);
        lv_label_set_text(content, "no S:/README.TXT?");
        lv_obj_set_style_text_color(status, lv_color_hex(0xFF5050), 0);
    } else {
        char data[64];
        uint32_t br = 0;
        r = lv_fs_read(&f, data, sizeof(data) - 1, &br);
        lv_fs_close(&f);

        data[br] = 0;
        for (uint32_t i = 0; i < br; i++) {
            if (data[i] < 0x20 || data[i] > 0x7E) data[i] = '.';
        }

        lv_label_set_text_fmt(status, "lv_fs_read: %d bytes (res=%d)", (int)br, (int)r);
        lv_label_set_text(content, data);
    }

    /* === heartbeat label to prove LVGL render loop alive === */
    lv_obj_t * tick = lv_label_create(scr);
    lv_obj_set_style_text_color(tick, lv_color_hex(0x5AC8FA), 0);
    lv_obj_align(tick, LV_ALIGN_BOTTOM_LEFT, 4, -4);

    uint32_t n = 0;
    char tickbuf[16];
    while (1) {
        n++;
        sprintf(tickbuf, "tick %lu", (unsigned long)n);
        lv_label_set_text(tick, tickbuf);
        lv_timer_handler();
        delay(50);
    }
}
