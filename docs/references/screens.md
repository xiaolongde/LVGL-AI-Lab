---
type: reference
project: LVGL-AI-Lab
updated: 2026-05-01
topic: 屏选项对比 + IC 关键参数
---

# 三块屏对比 + 选型推荐

> KIT 自带 3 块屏的资料 + Examples（实际物理屏数量取决于你订的 KIT 版本——优先确认手上**实物**是哪一块）。

## 对比表

| 屏 | 物理尺寸 | 分辨率 | 控制器 | KIT example | 资料路径 |
|---|---|---|---|---|---|
| 0.96" | 0.96" | **160×80** | ST7735S | `Examples/LCD 0.96inch BOE/` | `KIT/4. DATASHEET/屏幕技术资料/0.96 inch/` |
| 1.14" | 1.14" | **240×135** | ST7789V3 | `Examples/LCD 1.14inch/` | `KIT/4. DATASHEET/屏幕技术资料/1.14 inch/` |
| 1.47" | 1.47" | _待查_ | _待查_ | `Examples/LCD 1.47inch/` | （KIT 屏资料目录里**没有** 1.47 子目录）|

⚠ **注意**：之前文档里假设的"240x240"分辨率是错的。KIT 自带屏没有方屏 / 240x240 选项。1.14" 是 240x135，1.47" 待查（看 example tftlcd.h 的宏定义）。

## 选型推荐：1.14" ST7789V3 ✓

理由：
- **分辨率最高**（240x135）—— 表盘视觉信息量足
- **物理尺寸像手表**（手表屏一般 1.2-1.4"）
- ST7789 在 LVGL 社区资料**最厚**，AI 调起来阻力小
- KIT 自带 `LCD 1.14inch/` example，包含 ST7789 init sequence + 字库 + 简单 GUI（即可参考也是"AI 抄 vs 自写"的对照素材）

退路：如果实物只有 0.96"（ST7735S，160x80），方法论结论会偏向"AI 在更小屏上能不能优雅降级"——也是有价值的方向。

## ST7789V3（1.14" 屏）关键参数
- 接口：SPI 4-wire（CLK + MOSI + DC + CS），可选 8/16-bit 并口（板上没用）
- 颜色深度：**RGB565**（16 bit/pixel）—— LVGL `LV_COLOR_DEPTH=16`
- 显示 RAM：240×320×16bit（控制器内部，不占 MCU SRAM），实际可见区按 1.14" 模组裁到 240×135
- 时钟频率：最高 **62.5 MHz**（GD32F303 SPI0 在 60 MHz APB 上分频后远低于此，不会撞顶）
- 复位时序：硬复位 RST 拉低 ≥10us → 拉高 → 等 ≥120ms → 软复位 SWRESET → 等 ≥120ms → 配置寄存器 → SLPOUT → DISPON
- 主要寄存器：CASET (0x2A) 列地址、RASET (0x2B) 行地址、RAMWR (0x2C) 写显存
- Datasheet: `KIT/4. DATASHEET/屏幕技术资料/1.14 inch/ST7789V3_SPEC_Preliminary_V0.0_200102(1).pdf`
- 现成 init 代码: `KIT/3. SDK/extracted/GD32F303RCT6 Demo/Examples/LCD 1.14inch/Components/Devices/TFTLCD/tftlcd.c`

## ST7735S（0.96" 屏）关键参数
- 接口：SPI 4-wire
- 颜色深度：可选 RGB444 / RGB565 / RGB666，默认用 RGB565
- 显示 RAM: 162×132（裁到 160x80）
- 复位时序类似 ST7789
- Datasheet: `KIT/4. DATASHEET/屏幕技术资料/0.96 inch/ST7735S 芯片手册.pdf`
- 现成 init 代码: `KIT/3. SDK/extracted/GD32F303RCT6 Demo/Examples/LCD 0.96inch BOE/Components/Devices/TFTLCD/`

## LVGL partial buffer 估算（按 240×135 RGB565）
- 整屏: 240×135×2B = **64.8 KB** > 48 KB SRAM —— 整屏 framebuffer **不可行**
- 1/10 屏 partial buffer: ≈ 6.5 KB —— OK，留 41 KB 给应用 + 栈
- 推荐起手：**partial buffer 6KB**（双 buffer 关掉省 RAM）
- 0.96" (160×80) 同算: 整屏 25.6 KB，可整屏 framebuffer，但意义不大（LVGL 单 buffer + flush_cb 模式仍优）

## 实践笔记

（执行任务时遇到的 quirk / 反直觉点写在这里）

- _空_
