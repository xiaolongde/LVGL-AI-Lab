---
type: session-post
task: sa-lcd-bringup
pre: 20260502-0913-sa-lcd-bringup-pre.md
ended: 2026-05-02T09:29:28+08:00
outcome: success
---

## 实际遇到

**两个用户澄清把 AI 从错误路径拉回来 2 次**：

1. **我先误判 SPI 引脚**（references 老版本写"SPI0 remap"）→ KIT 代码实测用 SPI2，更正
2. **用户先说 IIC**，我刚 pivot 到要找 I2C OLED → 用户立即修正"不是 IIC，是 4 线 SPI，1.47" 不是 1.14""——AI 抄 1.14 文件作废，重新抄 1.47

第二次拉回来时我已经 copy 了 1.14" 全套文件 + 改了 main.c。pivot 到 1.47" 只需 overwrite tftlcd.{c,h}（其他文件 spi/usart/font/alientek_log 1.14 与 1.47 共享）。**AI 在用户澄清后 30 秒内完成路径切换**——这是个 datapoint：浪费的工作量 ≈ 1 分钟（拷贝时间），不是几小时。

第一次 build:
```
Flash 25504 B / 256 KB (9.73%)
RAM    4720 B /  48 KB (9.60%)
```
KIT 字模 (tft_font.h ~72KB 头文件，但实际链接进来的字模约 22KB) 是 Flash 大头。

烧录 + 复位后：
- ✅ 屏背光亮
- ✅ 4 行不同颜色文字正确显示（青/绿/白/黄）
- ✅ "tick=N" 心跳每秒递增
- ✅ RGB LED 也在闪（最初版本）
- 用户视觉 sign-off

**用户额外要求**：删 LED 闪。改 main.c 移除 gd_led_toggle 调用，gpio_config 已把 LED 默认置 HIGH = 共阳 RGB 灯熄状态。重 build → flash → 复位。Flash 减到 25420 B (-84 B)。

## 修复路径

无技术修复——pre 7 个卡点都没真触发：
- PA15 JTAG 关闭：gpio_config 已含
- PB7 PMOS 极性：低电平=导通=屏上电，KIT 代码自洽
- Reset 时序：tftlcd.c 内部已正确等 120ms
- 颜色端序：默认正确
- USE_HORIZONTAL=2 = 320x172 横屏（注意 1.47" 不是 240x135）
- main.h typedef u8/u16：从 KIT 1.14" Core/Inc/main.h 抄过来，typedef 齐
- size 突破：远未到阈值

唯一"修复"是用户澄清后切 1.47" tftlcd 文件。

## 学到的（方法论素材）

**新 lessons 候选**：

1. **AI 会 over-commit 到错误前提**：第一次 SPI 引脚我从 schematic 归纳成"SPI0 remap"是合理的（PB3/PB5 在 GD32F303 上确实可被 SPI0 用），但**没核对 KIT 实际代码**。验证 KIT 代码是个零成本动作（grep `RCU_SPI` 即知），AI 应该把"读 schematic + 归纳" 与 "查 vendor 例子" 都做一遍再下结论。Lessons 候选: `lessons/cross-check-schematic-with-vendor-code.md`

2. **早问澄清 vs 晚发现错误的成本曲线**：用户最初说"接上 LCD 基于 SPI"——我假设 1.14"（references 默认推荐）。**没问"哪个尺寸"的 1 秒钟代价**，是后面拷错文件的 1 分钟代价。但 1 秒钟问也会打断 momentum。trade-off：**明确多种 KIT 选项时（0.96 / 1.14 / 1.47）应该问**；只有一个时（如 LED）不需要。Lessons 候选: `lessons/when-to-clarify-vs-default.md`

3. **vendor SDK 同 controller 不同 panel 的差别集中在 2 处**：(a) `LCD_Address_Set` 偏移 (b) Init sequence 的 Vcom/Gamma 寄存器值。其余（spi.c, usart, font, application API）共享。这是个高复用度模式：v0.2 如果换屏，只需替换 tftlcd.{c,h}，其他不动。Lessons 候选: `lessons/st7789-panel-swap-pattern.md`

4. **AI 切错前提到 pivot 的速度**：从"用户说 IIC"到"build 1.47" SPI 通过 + 烧录 + 屏亮"用了约 5 分钟。这个 datapoint 对 capability report 重要：**AI 在被纠正后 reset 速度快**，前提是基础设施（CMake、烧录链）已就位。

5. **5 处 -Wchar-subscripts / -Wcomment 警告来自 KIT 代码**：`asc2_1206[chr][t]` 用 char 当 array index 在 -Wall 下警告（chr 可能为负值）。KIT 代码质量松散是个客观事实，未来若 lib 化要清理。当前不挡路，留 v0.2。

## 结果归档

- 屏 1.47" ST7789V3 320x172 SPI2 上电 + init + 文字显示 ✅
- 用户视觉 sign-off：文字正确 / 颜色正确 / tick 心跳活
- LED 按用户要求移除（main 循环纯屏 heartbeat）
- references/screens.md 1.47" 部分填实（替原"待查"）
- references/board.md SPI 分配修正（SPI0 remap → SPI2 default）
- 新文件: src/bsp/spi.{c,h}, src/main.h, src/devices/lcd/{tftlcd.c, tftlcd.h, tft_font.h, alientek_log.h}
- 新 build size: Flash 25420B (9.70%) / RAM 4720B (9.60%)

## 下一步

跳过原 plan 的 sub-task 2 (host mock 单测，premature for v0.1)。直接进 sb-lvgl-port：LVGL submodule + lv_conf 裁剪 + lv_port_disp 写 flush_cb (用 KIT 的 `LCD_Address_Set` + 批量 SPI 写) + hello world。
