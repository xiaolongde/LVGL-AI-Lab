/*!
    \file    hw_host.c
    \brief   Host PC simulator (Windows native backend) hw_* implementation
*/
#ifdef BUILD_HOST_SIM

#include "hw.h"
#include "lvgl.h"
#include "src/drivers/windows/lv_windows_display.h"
#include <stdio.h>
#include <windows.h>

int hw_boot(void)
{
    /* nothing to bring up on host; LVGL display init is later */
    printf("[host] boot OK\n");
    return 0;
}

void hw_boot_log(int row, const char * msg)
{
    (void)row;
    printf("[host] %s\n", msg);
}

void hw_install_font(void)
{
    /* the .bin lives in the cwd next to the .exe (cmake POST_BUILD copies it) */
    FILE * fp = fopen("./montserrat_28.bin", "rb");
    if (fp) {
        fclose(fp);
        printf("[host] font already at ./montserrat_28.bin\n");
    } else {
        printf("[host] WARN: ./montserrat_28.bin missing — lv_binfont_create will fail\n");
    }
}

void hw_lv_disp_init(void)
{
    /* 320x172 (matching ST7789V3 panel), zoom 200% so window is readable on PC */
    lv_windows_create_display(L"LVGL-AI-Lab Host Sim (320x172 @2x)",
                              320, 172,
                              200,    /* zoom level: 200% */
                              true,   /* allow DPI override */
                              true);  /* simulator mode (not resizable) */
}

void hw_lv_fs_init(void)
{
    /* lv_fs_stdio is auto-registered by lvgl when LV_USE_FS_STDIO=1
       via the lv_init() path; nothing to do here. */
}

const char * hw_font_path(void)
{
    /* both targets use "S:/montserrat_28.bin"; on host this resolves through
       LV_FS_STDIO_LETTER='S' + LV_FS_STDIO_PATH="./" → ./montserrat_28.bin */
    return "S:/montserrat_28.bin";
}

void hw_delay(uint32_t ms) { Sleep(ms); }

void hw_get_time(uint8_t * hh, uint8_t * mm, uint8_t * ss)
{
    SYSTEMTIME t;
    GetLocalTime(&t);
    *hh = (uint8_t)t.wHour;
    *mm = (uint8_t)t.wMinute;
    *ss = (uint8_t)t.wSecond;
}

#endif /* BUILD_HOST_SIM */
