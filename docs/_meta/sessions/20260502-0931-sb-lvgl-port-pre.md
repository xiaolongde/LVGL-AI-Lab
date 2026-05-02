---
type: session-pre
task: sb-lvgl-port
backlog_item: card2-lvgl-watch-face
started: 2026-05-02T09:31:04+08:00
---

## 任务理解

把 LVGL v9.5.0 跑在 GD32F303 + ST7789V3 1.47" 320x172 上。屏 driver (`tftlcd.c`) 已经能 init + Address_Set + 写半字像素，正好作为 LVGL 的"硬件层"。本任务在 LVGL 与 tftlcd 之间架一层 `lv_port_disp.c`，让 LVGL partial buffer 通过 `flush_cb` 推到屏。

成果：屏上居中显示 `LVGL v9.5.0` 字样（lv_label）+ tick 心跳，证明完整链路通。

## 计划

### 目录布局
```
external/lvgl/                     ← submodule pinned to v9.5.0
src/lvgl_port/
├── lv_conf.h                      ← 项目级 LVGL 配置（裁掉所有不用 widget）
├── lv_port_disp.c/h               ← flush_cb wraps tftlcd
└── lv_port_tick 用现有 systick    ← 改 SysTick_Handler 加 lv_tick_inc(1)
```

### 步骤
1. **lv_conf.h 关键项**：
   - `LV_COLOR_DEPTH 16`（RGB565）
   - `LV_COLOR_16_SWAP 1`（大端字节序——半字 SPI 发送 MSB 在前）
   - 关掉所有 demo / theme / 复杂 widget；只留 `LV_USE_LABEL`、`LV_USE_OBJ`
   - `LV_FONT_MONTSERRAT_14` 启用一个内置字体
   - `LV_MEM_SIZE` ≈ 8KB（LVGL 内部堆）
   - `LV_USE_LOG 0`
   - `LV_USE_OS = LV_OS_NONE`
2. **lv_port_disp.c**：~8KB partial buffer + flush_cb wraps `LCD_Address_Set` + `SPI2_WriteBytes`
3. **SysTick 接 lv_tick_inc**：改 `gd32f30x_it.c` 或 `systick.c` 的 SysTick_Handler 加 `lv_tick_inc(1)`
4. **main.c 重写**：删旧 LCD demo，加 lv_init + lv_port_disp_init + lv_label_create + lv_obj_center + while(lv_timer_handler + delay(5))
5. **CMakeLists**：add_library(lvgl STATIC GLOB external/lvgl/src/*.c) + include external/lvgl + src/lvgl_port + LV_CONF_INCLUDE_SIMPLE
6. cmake build → flash → reset → 用户看屏
7. session post + commit

## 卡点预判

1. **LVGL v9 文件超多**：第一次 build 可能 30-60 秒
2. **lv_conf.h 配错 → 编译错或 hardfault**（LV_USE_OS / LV_TICK_CUSTOM / LV_MEM_SIZE 配错最常见）
3. **partial buffer flush 时 SPI 串扰**：先单 buffer 简单跑
4. **字节序**：ST7789V3 期望大端 RGB565，LVGL 默认小端。`LV_COLOR_16_SWAP=1` 是颜色不对的最常见根因
5. **size 突破 200KB**：LVGL v9 裁剪后预估 50-80KB，加现有 25KB ≈ 100KB，安全
6. **lv_init 顺序错就 hardfault**
7. **AI 不熟 v9 API**：v9 (2024+) 改了很多（lv_disp_drv_t → lv_display_t、lv_scr_act() → lv_screen_active()）。会查 lvgl/examples/porting 对照
8. **包含 lvgl.h 后 vendor 库符号冲突**：可能与 newlib 的某些定义冲突。需要时 `LV_USE_BUILTIN_*` 关掉

## 验收

- [ ] cmake --build 通过
- [ ] Flash < 150KB
- [ ] flash + reset → 屏居中显示 "LVGL v9.5.0"
- [ ] 不出现 hardfault
- [ ] smoke 阶段 1+2 PASS
- [ ] post 含 ≥3 lessons 候选
