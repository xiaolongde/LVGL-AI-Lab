/*!
    \file    main.c
    \brief   v0.2 sj-lv-font-conv-pipeline: load Montserrat 28px from SD via lv_binfont_create
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
#include "src/font/binfont_loader/lv_binfont_loader.h"

extern void lv_fs_fatfs_init(void);

/* embedded font blob — auto-installed to SD on first boot */
extern const uint8_t  montserrat_28_bin_data[];
extern const uint32_t montserrat_28_bin_size;

static FATFS s_fs;
static char  s_path[4];

static void install_font_if_missing(void)
{
    FIL fp;
    FRESULT fr = f_open(&fp, "0:/montserrat_28.bin", FA_READ);
    if (fr == FR_OK) {
        f_close(&fp);
        LCD_ShowString(0, 16, 320, 16, 12, "font: already on SD");
        return;
    }
    LCD_ShowString(0, 16, 320, 16, 12, "font: installing to SD...");
    fr = f_open(&fp, "0:/montserrat_28.bin", FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        char b[40]; sprintf(b, "install open fail: %d", (int)fr);
        POINT_COLOR = RED;
        LCD_ShowString(0, 30, 320, 16, 12, b);
        return;
    }
    UINT bw = 0;
    fr = f_write(&fp, montserrat_28_bin_data, montserrat_28_bin_size, &bw);
    f_close(&fp);
    char b[60];
    if (fr == FR_OK && bw == montserrat_28_bin_size) {
        POINT_COLOR = GREEN;
        sprintf(b, "install OK: %u B", bw);
    } else {
        POINT_COLOR = RED;
        sprintf(b, "install FAIL: w=%d br=%u/%lu", (int)fr, bw, (unsigned long)montserrat_28_bin_size);
    }
    LCD_ShowString(0, 30, 320, 16, 12, b);
    POINT_COLOR = WHITE;
}

static const char * sd_err_str(sd_error_enum e)
{
    switch (e) {
        case SD_OK: return "OK";
        case SD_CMD_RESP_TIMEOUT: return "CMD_TIMEOUT";
        case SD_DATA_TIMEOUT: return "DATA_TIMEOUT";
        default: return "ERR";
    }
}

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

    POINT_COLOR = WHITE;
    BACK_COLOR  = BLACK;
    LCD_ShowString(0, 0, 320, 16, 12, "v0.2 sj: SD bin font");

    char buf[80];

    sd_card_info_struct info;
    sd_error_enum sderr = sd_full_init(&info);
    if (sderr != SD_OK) {
        POINT_COLOR = RED;
        sprintf(buf, "SD FAIL: %s", sd_err_str(sderr));
        LCD_ShowString(0, 16, 320, 16, 12, buf);
        while (1) { delay(500); }
    }

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

    /* hands-off: install font to SD on first boot */
    install_font_if_missing();

    lv_init();
    lv_port_disp_init();
    lv_fs_fatfs_init();

    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0A0E1A), 0);

    /* boot status (small font) */
    lv_obj_t * status = lv_label_create(scr);
    lv_obj_set_style_text_color(status, lv_color_hex(0xD4A373), 0);
    lv_obj_align(status, LV_ALIGN_TOP_LEFT, 4, 4);

    /* === pre-flight: list root dir + verify font file via lv_fs === */
    {
        lv_obj_t * dl = lv_label_create(scr);
        lv_obj_set_style_text_color(dl, lv_color_hex(0xFFFF00), 0);
        lv_obj_align(dl, LV_ALIGN_TOP_LEFT, 4, 22);
        lv_obj_set_width(dl, 312);
        lv_label_set_long_mode(dl, LV_LABEL_LONG_WRAP);

        DIR dir; FILINFO fno; FRESULT pr = f_opendir(&dir, "0:/");
        char list[160]; int off = snprintf(list, sizeof(list), "ls 0:/  ");
        if (pr == FR_OK) {
            int n = 0;
            while (n < 6) {
                if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) break;
                int rem = sizeof(list) - off - 1;
                int w = snprintf(list + off, rem, "[%s] ", fno.fname);
                if (w < 0 || w >= rem) break;
                off += w; n++;
            }
            f_closedir(&dir);
        }
        lv_label_set_text(dl, list);

        /* Direct lv_fs probe + dump first 8 bytes */
        lv_obj_t * fl = lv_label_create(scr);
        lv_obj_set_style_text_color(fl, lv_color_hex(0xFFFF00), 0);
        lv_obj_align(fl, LV_ALIGN_TOP_LEFT, 4, 60);
        lv_obj_set_width(fl, 312);
        lv_label_set_long_mode(fl, LV_LABEL_LONG_WRAP);
        lv_fs_file_t probe;
        lv_fs_res_t fr = lv_fs_open(&probe, "S:/montserrat_28.bin", LV_FS_MODE_RD);
        static char fbuf[160];
        if (fr == LV_FS_RES_OK) {
            uint8_t hdr[12];
            uint32_t br = 0;
            lv_fs_read(&probe, hdr, 12, &br);
            uint32_t sz = 0;
            lv_fs_seek(&probe, 0, LV_FS_SEEK_END);
            lv_fs_tell(&probe, &sz);
            lv_fs_close(&probe);
            sprintf(fbuf, "fs OK sz=%lu hdr=%02X%02X%02X%02X %c%c%c%c v=%02X%02X%02X%02X",
                    (unsigned long)sz,
                    hdr[0], hdr[1], hdr[2], hdr[3],
                    hdr[4], hdr[5], hdr[6], hdr[7],
                    hdr[8], hdr[9], hdr[10], hdr[11]);
        } else {
            sprintf(fbuf, "lv_fs_open ERR %d", (int)fr);
        }
        lv_label_set_text(fl, fbuf);
    }

    /* === load Montserrat 28px from SD via lv_binfont === */
    lv_font_t * font_big = lv_binfont_create("S:/montserrat_28.bin");

    if (font_big == NULL) {
        lv_label_set_text(status, "lv_binfont_create FAIL");
        lv_obj_set_style_text_color(status, lv_color_hex(0xFF5050), 0);
    } else {
        lv_label_set_text(status, "lv_binfont 28px LOADED");
        lv_obj_set_style_text_color(status, lv_color_hex(0x4AD98E), 0);

        /* === BIG time text using SD-loaded font === */
        lv_obj_t * time_lbl = lv_label_create(scr);
        lv_obj_set_style_text_font(time_lbl, font_big, 0);
        lv_obj_set_style_text_color(time_lbl, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(time_lbl, "12:34");
        lv_obj_align(time_lbl, LV_ALIGN_CENTER, 0, 0);

        /* small subtitle */
        lv_obj_t * sub = lv_label_create(scr);
        lv_obj_set_style_text_color(sub, lv_color_hex(0x808080), 0);
        lv_label_set_text(sub, "loaded from S:/montserrat_28.bin");
        lv_obj_align(sub, LV_ALIGN_CENTER, 0, 24);
    }

    /* heartbeat */
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
