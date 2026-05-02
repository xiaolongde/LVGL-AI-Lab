---
type: session-pre
task: si-lvgl-fs-drv
backlog_item: card3-sd-fatfs-gui-resources
started: 2026-05-02T20:17:17+08:00
---

## 任务理解

v0.2 Card 3 第三站：把 LVGL 重新加回 build（sg/sh 暂删了），桥接 LVGL `lv_fs_drv_t` 到 FatFs，注册 'S:' 字母驱动器。让 LVGL 能用 lv_fs_open / lv_fs_read 读 SD 卡。这步是 sj/sk（大字字体从 SD 加载）的前置条件。

## 计划

1. 启 `LV_USE_FS_FATFS=1`, `LV_FS_FATFS_LETTER='S'`, `LV_FS_FATFS_PATH="0:"`（lvgl 自带 lv_fs_fatfs.c 不用自写）
2. CMakeLists: lvgl 加 fatfs include 路径（lv_fs_fatfs.c 要 include "ff.h"）
3. main.c 整合：sd_full_init pipeline → FATFS_LinkDriver → f_mount → lv_init → lv_port_disp_init → `lv_fs_fatfs_init()` 注册 S:
4. 验证：`lv_fs_open("S:/README.TXT", LV_FS_MODE_RD)` + lv_fs_read → label 显示
5. heartbeat（label 或 bar）证 LVGL render loop 不阻塞

## 卡点预判

1. **Flash overflow**：LVGL ~210K + FatFs ~12K + sd ~10K + KIT lcd ~5K + lv_fs_fatfs ~5K → 240+，紧。可能要砍 LVGL widget。
2. **`lv_fs_fatfs.c` 找不到 ff.h**：lvgl 静态库没包 fatfs include path。改 CMakeLists。
3. **lv_init() 后 lv_tick 没推**：sb 时已确认 SysTick_Handler 调 lv_tick_inc。
4. **路径前缀拼错**：lv_fs strip "S:" → real_path 是 "/file.txt"；lv_fs_fatfs 拼 LV_FS_FATFS_PATH+real → "0:/file.txt"。要让 LV_FS_FATFS_PATH = "0:" 不带斜杠。
5. **dir_open / dir_read 在 LVGL 里 vs FatFs 接口差异**：LVGL 也走 f_opendir / f_readdir，应直通。

## 验收

- [ ] LV_USE_FS_FATFS=1 生效，build 通过
- [ ] Flash < 256K
- [ ] reset → SD init OK + 屏出 LVGL UI（玫瑰金标题）
- [ ] lv_fs_open + lv_fs_read 返 OK，文件内容显在 label
- [ ] tick label 持续递增（render loop 活）
- [ ] post 含 lessons 候选

## 元注释（autonomous mode）

- LV_USE_FS_FATFS=1 是 LVGL 内置驱动，不用自写 lv_port_fs_sd.c → self-recommend
- size pressure 时按"先 widget 关 → 再 LFN 关 → 再 LOG_LEVEL 关"顺序
- ≥ 3 次失败 → BFS 回 sh post 看 FatFs 路径处理；卡点前预判 #1 Flash 是高风险
