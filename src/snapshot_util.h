/*!
    \file    snapshot_util.h
    \brief   Snapshot LVGL screen to a PPM file (binary P6, RGB888).
             PPM 是最简单的无压缩图像格式，无需 libpng/zlib —
             用 Python (Pillow / 自写) 比较两张 PPM 一行 diff 即可。
*/
#ifndef SNAPSHOT_UTIL_H
#define SNAPSHOT_UTIL_H

/* dump current active screen to `path` as PPM (P6 binary RGB888).
   Returns 0 on success, non-zero on failure. */
int snapshot_screen_to_ppm(const char * path);

#endif
