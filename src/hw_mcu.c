/*!
    \file    hw_mcu.c
    \brief   MCU hw_* implementation (GD32F303RC + ST7789V3 + SD/FATFS)
*/
#ifndef BUILD_HOST_SIM

#include "hw.h"
#include "gd32f30x.h"
#include <stdio.h>

#include "systick.h"
#include "gpio.h"
#include "spi.h"
#include "tftlcd.h"
#include "sdcard.h"

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "lv_port_disp.h"

extern void lv_fs_fatfs_init(void);
extern const uint8_t  montserrat_28_bin_data[];
extern const uint32_t montserrat_28_bin_size;

static FATFS s_fs;
static char  s_path[4];

static sd_error_enum sd_full_init(sd_card_info_struct * info)
{
    sd_error_enum err = sd_init();
    if (err == SD_OK) err = sd_card_information_get(info);
    if (err == SD_OK) err = sd_card_select_deselect(info->card_rca);
    if (err == SD_OK) err = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
    if (err == SD_OK) err = sd_transfer_mode_config(SD_POLLING_MODE);
    return err;
}

int hw_boot(void)
{
    systick_config();
    gpio_config();
    spi_lcd_init();
    LCD_Init();
    LCD_Fill(0, 0, LCD_Width - 1, LCD_Height - 1, BLACK);
    POINT_COLOR = WHITE; BACK_COLOR = BLACK;

    sd_card_info_struct info;
    if (sd_full_init(&info) != SD_OK) return -1;
    if (FATFS_LinkDriver(&sd_diskio_drv, s_path) != 0) return -2;
    if (f_mount(&s_fs, s_path, 1) != FR_OK) return -3;
    return 0;
}

void hw_boot_log(int row, const char * msg)
{
    LCD_ShowString(0, row, 320, 16, 12, (char *)msg);
}

void hw_install_font(void)
{
    FIL fp;
    if (f_open(&fp, "0:/montserrat_28.bin", FA_READ) == FR_OK) { f_close(&fp); return; }
    if (f_open(&fp, "0:/montserrat_28.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    UINT bw = 0;
    f_write(&fp, montserrat_28_bin_data, montserrat_28_bin_size, &bw);
    f_close(&fp);
}

void hw_lv_disp_init(void) { lv_port_disp_init(); }
void hw_lv_fs_init(void)   { lv_fs_fatfs_init(); }

const char * hw_font_path(void) { return "S:/montserrat_28.bin"; }

void hw_delay(uint32_t ms) { delay(ms); }

#endif /* !BUILD_HOST_SIM */
