/* lv_port_disp.c — LVGL v9 display port for ST7789V3 1.47" 320x172 via SPI2
 *
 * 桥接：LVGL flush_cb (px_map RGB565 8KB partial buffer) → KIT 的 LCD_Address_Set + SPI2_WriteBytes
 *
 * 平台：GD32F303RCT6 + ST7789V3 1.47" 横屏
 * 假设：spi_lcd_init() + LCD_Init() 已先于 lv_port_disp_init() 调用过
 */

#include "lv_port_disp.h"
#include "lvgl.h"
#include "tftlcd.h"
#include "spi.h"

#define DISP_HOR_RES   320
#define DISP_VER_RES   172
#define BYTE_PER_PX    2     /* RGB565 */

/* 8 KB partial buffer ≈ 12 行 320 像素 */
#define BUF_LINES      12
#define BUF_PIXELS     (DISP_HOR_RES * BUF_LINES)
#define BUF_BYTES      (BUF_PIXELS * BYTE_PER_PX)

static uint8_t s_disp_buf[BUF_BYTES] __attribute__((aligned(4)));

static void disp_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    /* RGB565 字节序：LVGL 内存 little-endian (low byte first)，ST7789 期望 big-endian (high byte first)
     * 用 LVGL 内置 helper 原地 swap（partial mode 下 buffer 会被 LVGL 重用，原地修改 OK）。 */
    uint32_t px_count = (uint32_t)(area->x2 - area->x1 + 1)
                      * (uint32_t)(area->y2 - area->y1 + 1);
    lv_draw_sw_rgb565_swap(px_map, px_count);

    /* 设置写窗口 */
    LCD_Address_Set((u16)area->x1, (u16)area->y1, (u16)area->x2, (u16)area->y2);

    /* 推像素：area 像素数 × 2 字节，DC=高（数据） */
    LCD_DC(1);
    SPI2_WriteBytes(px_map, px_count * BYTE_PER_PX);

    /* 通知 LVGL flush 完成 */
    lv_display_flush_ready(disp);
}

void lv_port_disp_init(void)
{
    lv_display_t * disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush_cb);
    lv_display_set_buffers(disp, s_disp_buf, NULL, sizeof(s_disp_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
}
