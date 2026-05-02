---
type: reference
project: LVGL-AI-Lab
updated: 2026-05-01
topic: 屏选项对比 + IC 关键参数
---

# 三块屏对比 + 选型推荐

> KIT 自带 3 块屏的资料 + Examples（实际物理屏数量取决于你订的 KIT 版本——优先确认手上**实物**是哪一块）。

## 对比表

| 屏 | 物理尺寸 | 分辨率（横屏） | 控制器 | KIT example | 资料路径 |
|---|---|---|---|---|---|
| 0.96" | 0.96" | **160×80** | ST7735S | `Examples/LCD 0.96inch BOE/` | `KIT/4. DATASHEET/屏幕技术资料/0.96 inch/` |
| 1.14" | 1.14" | **240×135** | ST7789V3 | `Examples/LCD 1.14inch/` | `KIT/4. DATASHEET/屏幕技术资料/1.14 inch/` |
| **1.47" ✅ 实际选用** | 1.47" | **320×172** | ST7789V3 | `Examples/LCD 1.47inch/` | （KIT 资料目录无独立子目录；ST7789V3 共用 1.14" datasheet）|

⚠ **重要**：之前文档里假设的"240x240"分辨率是错的；KIT 自带屏无方屏选项。

## 实际选用：1.47" ST7789V3 (320x172) ✓

用户实物接的就是这块。

- **分辨率 320x172** 横屏，比 1.14" (240x135) 视觉宽 33% / 高 27%，表盘信息空间更舒服
- **驱动 IC** 同为 ST7789V3，与 1.14" 共用 datasheet
- **接口**: 4 线 SPI（CLK + MOSI + DC + CS），LCD_RST + LCD_PWR(PMOS 控屏供电)
- KIT 自带 `Examples/LCD 1.47inch/`：driver 代码 + alientek 字库 + demo

### 1.47" vs 1.14" 主要差别（KIT example 角度）
- `tftlcd.h`: `LCD_Width=320 LCD_Height=172`（vs 240/135）
- `tftlcd.c::LCD_Address_Set`: 4 个方向的 RAM 偏移（1.47" 用 +34，1.14" 用 +52/+40/+53/+40 等）—— 因为不同物理 panel 在 ST7789 320x320 internal RAM 里的可见窗口位置不同
- `tftlcd.c::LCD_Init`: Vcom (0x35 vs 0x19)、gamma 寄存器值若干处不同
- 其他文件（spi/usart/font/alientek_log）相同

退路：如果换 0.96" 或 1.14"，driver 替换为对应 KIT example 即可，应用层无需改（API 层抽象同）。

## ST7789V3（共用，1.14"/1.47"）关键参数
- 接口：SPI 4-wire（CLK + MOSI + DC + CS），可选 8/16-bit 并口（板上没用）
- 颜色深度：**RGB565**（16 bit/pixel）—— LVGL `LV_COLOR_DEPTH=16`
- 显示 RAM：320×320×16bit（控制器内部，不占 MCU SRAM），实际可见区按 panel 模组裁
- 时钟频率：最高 **62.5 MHz**（GD32F303 SPI2 在 60 MHz APB 上 /8 = 7.5 MHz，远低于上限）
- 复位时序：硬复位 RST 拉低 ≥10us → 拉高 → 等 ≥120ms → 软复位 SWRESET → 等 ≥120ms → 配置寄存器 → SLPOUT → DISPON
- 主要寄存器：CASET (0x2A) 列地址、RASET (0x2B) 行地址、RAMWR (0x2C) 写显存
- Datasheet: `KIT/4. DATASHEET/屏幕技术资料/1.14 inch/ST7789V3_SPEC_Preliminary_V0.0_200102(1).pdf`
- 现成 init 代码: `KIT/.../Examples/LCD 1.47inch/Components/Devices/TFTLCD/tftlcd.c`

## ST7735S（0.96" 屏）关键参数
- 接口：SPI 4-wire
- 颜色深度：可选 RGB444 / RGB565 / RGB666，默认用 RGB565
- 显示 RAM: 162×132（裁到 160x80）
- Datasheet: `KIT/4. DATASHEET/屏幕技术资料/0.96 inch/ST7735S 芯片手册.pdf`
- 现成 init 代码: `KIT/.../Examples/LCD 0.96inch BOE/Components/Devices/TFTLCD/`

## LVGL partial buffer 估算（按 320×172 RGB565）
- 整屏: 320×172×2B = **110 KB** > 48 KB SRAM —— 整屏 framebuffer **不可行**
- 1/10 屏 partial buffer: ≈ 11 KB —— 紧但可行（把 lv_conf 关掉所有不用的 widget 后剩 30+ KB 给 buf + stack + heap）
- 推荐起手：**partial buffer 8KB**（≈ 1/13 屏，约 12 行）；double buffer 关掉省 RAM
- 比 1.14" (整屏 65KB / 6KB partial) 更紧，但仍 fit。

## 实践笔记
- _空_

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

## 旧估算（240×135，已过时——保留为历史）
（项目实际选了 1.47" 320×172，新估算见上节）
