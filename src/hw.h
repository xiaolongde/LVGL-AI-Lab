/*!
    \file    hw.h
    \brief   Hardware abstraction — MCU (GD32+ST7789+SD) vs Host (Win32)

    main.c is platform-agnostic; the two impls live in src/hw_mcu.c
    (linked for MCU build) and src/hw_host.c (linked for BUILD_HOST_SIM).
*/
#ifndef HW_H
#define HW_H

#include <stdint.h>

/* low-level boot: clock, peripherals, screen, SD/FATFS mount. Returns 0 on success. */
int  hw_boot(void);

/* show a one-line boot message before LVGL takes over (MCU: KIT TFTLCD;
   Host: stdout) */
void hw_boot_log(int row, const char * msg);

/* install embedded font blob to the storage (MCU: SD card; Host: noop —
   the .bin is already in cwd via cmake post-build) */
void hw_install_font(void);

/* register LVGL display (MCU: lv_port_disp_init; Host: lv_windows_create_display) */
void hw_lv_disp_init(void);

/* register LVGL filesystem driver (MCU: lv_fs_fatfs_init; Host: lv_fs_stdio is
   auto-registered when LV_USE_FS_STDIO=1, so this is a noop) */
void hw_lv_fs_init(void);

/* path passed to lv_binfont_create — both targets resolve "S:" via their
   respective lv_fs driver */
const char * hw_font_path(void);

/* sleep blocking for `ms` milliseconds */
void hw_delay(uint32_t ms);

/* current wall-clock time (24h). On MCU: backed by RTC LSE.
   On host: Windows local time. */
void hw_get_time(uint8_t * hh, uint8_t * mm, uint8_t * ss);

#endif /* HW_H */
