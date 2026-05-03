/*!
    \file    snapshot_util.c
    \brief   lv_snapshot_take → RGB565 → RGB888 → PPM
*/

#include "snapshot_util.h"
#include "lvgl.h"
#include <stdio.h>

int snapshot_screen_to_ppm(const char * path)
{
#if LV_USE_SNAPSHOT
    lv_obj_t * scr = lv_screen_active();
    /* take into RGB565 (matches our display + small) */
    lv_draw_buf_t * buf = lv_snapshot_take(scr, LV_COLOR_FORMAT_RGB565);
    if (!buf) return -1;

    int32_t w = (int32_t)buf->header.w;
    int32_t h = (int32_t)buf->header.h;
    uint16_t * px = (uint16_t *)buf->data;

    FILE * fp = fopen(path, "wb");
    if (!fp) { lv_draw_buf_destroy(buf); return -2; }

    /* PPM P6 header */
    fprintf(fp, "P6\n%d %d\n255\n", (int)w, (int)h);

    /* RGB565 → RGB888 row by row (PPM is byte-stream big-endian RGB) */
    for (int32_t y = 0; y < h; y++) {
        uint16_t * row = px + y * (buf->header.stride / 2);
        for (int32_t x = 0; x < w; x++) {
            uint16_t v = row[x];
            uint8_t r = (uint8_t)(((v >> 11) & 0x1F) * 255 / 31);
            uint8_t g = (uint8_t)(((v >>  5) & 0x3F) * 255 / 63);
            uint8_t b = (uint8_t)(( v        & 0x1F) * 255 / 31);
            uint8_t out[3] = { r, g, b };
            fwrite(out, 1, 3, fp);
        }
    }

    fclose(fp);
    lv_draw_buf_destroy(buf);
    return 0;
#else
    (void)path;
    return -100; /* snapshot not enabled in this build */
#endif
}
