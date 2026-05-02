---
type: reference
project: LVGL-AI-Lab
updated: 2026-05-01
topic: 板上外设 + 引脚映射
---

# GD32F303RxT6 V5.1 KIT — 板上外设 + 引脚映射

> 信息来源：`KIT/2. HDK/V5.1/Schematic.pdf`（已直接读图 + KIT Examples 中代码交叉验证）。
> 板风格：alientek 风（依据 SDK 文件 `alientek_log.h` 推测）。

## 板上有什么
- GD32F303RCT6 LQFP64 主控
- 8 MHz HSE 晶振 (XT1) + 22pF 起振电容
- **32.768 KHz LSE 晶振 (XT2)** + RTC 后备电池座（RTC1，含 1N4148 隔离）— RTC 直接可用
- TPS70933DBVR 5V→3V3 LDO（输入来自 USB Type-C VBUS）
- USB Type-C 接口（PA11=DM, PA12=DP，含 USBLC6-2SC6 ESD 保护）
- 16x16 共阳 RGB LED (D1)
- W25Q64 SPI Flash (8 MB)
- SD 卡槽（SD1，走 SDIO）
- LCD FPC10 接口（外接屏）
- 3 个按键: NRST、BOOT0、USER1
- SWD + USART1 共用 8-pin header (P1)
- 4 个定位孔 (JL1-4)

## 引脚映射全表（按外设组）

### LED（RGB，每个 1K 串阻）
| 颜色 | MCU 引脚 |
|---|---|
| R | PC0 |
| G | PC1 |
| B | PC2 |

### 按键
| 按键 | 引脚 | 备注 |
|---|---|---|
| NRST | NRST | 物理复位按键 |
| BOOT0 | BOOT0 | 启动模式选择（10K 下拉） |
| USER1 | PC13 | 用户按键 |

### LCD（FPC10，**SPI2 默认引脚**，单工写）
| 信号 | 引脚 | 方向 |
|---|---|---|
| LCD_SCL（SPI2_SCK） | PB3 | 输出（SPI AF） |
| LCD_SDA（SPI2_MOSI） | PB5 | 输出（SPI AF） |
| LCD_DC | PB4 | GPIO 输出（数据/命令；占用 SPI2_MISO 槽位但只做 GPIO） |
| LCD_RST | PB6 | GPIO 输出（屏复位） |
| LCD_CS | PA15 | GPIO 输出（SPI2_NSS 槽，但软件 NSS 模式当 GPIO 用）|
| LCD_PWR | PB7 | GPIO 输出（**PMOS 控屏供电**，**低电平导通=屏上电**） |
| LEDA | (FPC pin 1) | 背光阳极，板上接 3V3 |

⚠ **关键**：
- 这是 **SPI2 默认引脚**（GD32F303 SPI2 的 default mapping = PA15/PB3/PB4/PB5），**不是 SPI0 remap**——尽管 PB3/PB5 在 SPI0 remap 时也是这俩引脚（GD32 与 STM32F1 兼容）。KIT 代码用 `RCU_SPI2 + SPI2` 确认。
- LCD_DC 占用 PB4 = SPI2_MISO 槽位但纯做 GPIO，所以这是 3 线 SPI（CLK + MOSI + DC + CS），不能 readback。LVGL flush 是写单向，OK。
- PA15 是 JTAG-DP 默认（JTDI），用作 GPIO/SPI2 必须先 `gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE)` 关 JTAG（保留 SWD）—— 已在 RGB LED gpio.c 完成。
- SPI 配置: Mode 3 (CK_PL_HIGH_PH_2EDGE), MSB first, prescaler /8（120MHz/8/2=7.5MHz @ APB1 clock）。

### SPI Flash W25Q64（**SPI1 默认引脚**，8MB）
| 信号 | 引脚 |
|---|---|
| FLASH_CS | PB12（NSS 软件） |
| SPI1_SCK | PB13 |
| SPI1_MISO | PB14 |
| SPI1_MOSI | PB15 |

### SDIO（4-bit 模式）
| 信号 | 引脚 |
|---|---|
| SDIO_D0 | PC8 |
| SDIO_D1 | PC9 |
| SDIO_D2 | PC10 |
| SDIO_D3 | PC11 |
| SDIO_CK | PC12 |
| SDIO_CMD | PD2 |

### USB Type-C（USBD device）
| 信号 | 引脚 |
|---|---|
| USB_DM | PA11 |
| USB_DP | PA12 |
| CC1 detect | (CC1，5.1K 下拉) |
| CC2 detect | (CC2，5.1K 下拉) |

### SWD + 串口 调试 header (P1)
| Pin | 信号 | 引脚 |
|---|---|---|
| 1 | SWDIO | PA13 |
| 2 | SWDCLK | PA14 |
| 3 | RX (USART1) | PA10 |
| 4 | TX (USART1) | PA9 |
| 5 | NRST | NRST |
| 6 | 5V / 3V3 | (供电) |

### 时钟晶振
- HSE: PD0/PD1（OSC_IN_H/OSC_OUT_H），8 MHz
- LSE: PC14/PC15（OSC32_IN/OSC32_OUT），32.768 KHz

### VDDA / VBAT
- VDDA 通过 ferrite bead 600R 与 3V3 隔离 + 100nF 滤波
- VBAT 通过 RTC 后备电池 + 1N4148 隔离

## SPI 通道分配（**修正**：原 references 把 SPI0/SPI2 写反了）
| SPI | 用途 | 引脚 | 备注 |
|---|---|---|---|
| SPI2 (default) | LCD | PA15/PB3/PB5 + PB4(DC)+PB6(RST)+PB7(PWR) | 3-wire 写，软件 NSS |
| SPI1 (default) | W25Q64 Flash | PB12/13/14/15 | 4-wire，软件 NSS |
| SPI0 | 闲置 | 默认 PA4-7，板上未用 | 可用 |

## 实践笔记

（执行任务时遇到的 quirk / 反直觉点写在这里）

- _空_
