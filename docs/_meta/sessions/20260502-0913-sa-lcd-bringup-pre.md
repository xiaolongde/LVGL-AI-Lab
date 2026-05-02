---
type: session-pre
task: sa-lcd-bringup
backlog_item: card2-lvgl-watch-face
started: 2026-05-02T09:13:26+08:00
---

## 任务理解

把 KIT/Examples/LCD 1.14inch 的屏驱动整套抄进项目并烧录验证。屏接好了（用户已接），需要：
1. SPI bus 起来（**SPI2** 默认引脚 PB3/PB5/PA15，不是之前 references 写的 SPI0 remap——KIT 代码确认是 SPI2）
2. ST7789V3 init sequence 跑（120ms reset + SLPOUT + 配置寄存器 + DISPON）
3. 控制屏上显示一些 sanity 内容（清屏 + 字符串），用户视觉确认

## 计划

1. 修 `references/board.md`（SPI 分配错了：LCD 是 **SPI2 默认**，不是 SPI0 remap；W25Q64 是 **SPI1 默认**）
2. 复制文件：
   - `KIT/.../BSP/spi.c spi.h` → `src/bsp/spi.{c,h}`
   - `KIT/.../Devices/TFTLCD/{tftlcd.c, tftlcd.h, tft_font.h, alientek_log.h}` → `src/devices/lcd/`
3. CMakeLists.txt 添加 `src/devices/**/*.c` 到 sources，include `src/devices/lcd/`
4. 改写 main.c：删 LED 闪 demo，改成 `spi_lcd_init() → LCD_Init() → LCD_Clear(BLACK) → LCD_ShowString` 显示文字 + LED 心跳保留
5. cmake --build → flash → 用户看屏
6. 屏不亮 → 三阶诊断：(a) GPIO (b) SPI (c) ST7789V3 init 时序
7. session post + commit

## 卡点预判

1. **PA15 默认是 JTAG-DP**（JTDI），SPI2 NSS 用 PA15 必须先关 JTAG。RGB LED 的 gpio_config 已经做了 `gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE)`——保留。
2. **LCD_PWR (PB7) 通过 PMOS 控屏供电**：低电平导通。KIT spi.c `gpio_bit_reset(GPIOB, GPIO_PIN_7)` ✓ 自洽。
3. **Reset 时序**：tftlcd.c LCD_Init 内 RST 拉低 → 拉高 → 120ms。systick_config 已就位。
4. **颜色端序**：ST7789V3 默认 RGB565；MADCTL 配错可能 BGR/RGB 反。
5. **USE_HORIZONTAL=2** = 240x135 横屏。对得上 references。
6. **main.h 的 typedef u8/u16**：KIT 代码用 u8/u16/u32（不是 stdint），main.h 应该 typedef 了。复制 main.h 时检查，否则编译 fail。
7. **size 突破阈值**：tft_font.h 72KB（字模数据）；总 Flash 应该 < 100KB，离 200KB 限还远。

## 验收

- [ ] cmake --build 通过（含 SPI + TFTLCD + tft_font）
- [ ] flash 成功
- [ ] 用户视觉：屏背光亮 + 有文字 + 颜色循环
- [ ] smoke 阶段 1+2 PASS
- [ ] post 含 1-2 lessons 候选
