---
type: reference
project: LVGL-AI-Lab
updated: 2026-05-01
topic: MCU
---

# GD32F303RCT6 — 关键事实速查

> 详见 datasheet (`KIT/4. DATASHEET/GD32F303xx_Datasheet_Rev1.8.pdf`) + ref manual (`GD32F30x_User_Manual_CN_Rev2.8.pdf`)。

## 核心参数
- 内核: ARM Cortex-M4 + FPU（单精度），最高 **120 MHz**
- Flash: **256 KB** @ 0x08000000
- SRAM: **48 KB** @ 0x20000000
- 封装: LQFP64
- 与 STM32F103 寄存器**部分兼容**（GD 沿袭 F1 系列），但时钟树和某些外设不完全一致——**不能盲套 STM32 example，要查 GD ref manual**

## 时钟树（板上情况）
- HSE: **8 MHz** 外部晶振（XT1）→ PLL → 系统时钟 120 MHz
- LSE: **32.768 KHz** 外部晶振（XT2）→ RTC（板上已就绪，不用改板）
- LSI: 内部 ~40 KHz（备用，精度差）
- HSI: 内部 8 MHz RC（备用）

## 外设关键点（与意图相关）
- **SPI**: 3 个（SPI0/SPI1/SPI2）。本板 SPI0 remap 走 LCD（PB3/PB5），SPI2 走 W25Q64 flash（PB13-15）
- **RTC**: GD32F30x 沿袭 STM32F1 风格 = **32 位计数器型 RTC**，需软件转换 H:M:S（不像 F4/F7 自带 calendar）。BKP 寄存器可用于 magic 检测断电
- **GPIO**: 5 个 port (A/B/C/D 部分)，每 pin 可配置 IN/OUT/AF/Analog 等
- **SysTick**: ARM 内核标配，1ms tick 是 LVGL 的 `lv_tick_inc()` 来源
- **DMA**: 2 个控制器（DMA0 7 通道、DMA1 5 通道），可挂 SPI 加速 LCD flush（v0.2 用得上）
- **EXMC**: 外部存储器控制器（本 KIT 板没用到，但可挂 SDRAM/SRAM 扩 RAM——v0.2 备选）

## 标准外设库（KIT 自带）
路径: `KIT/3. SDK/extracted/GD32F303RCT6 Demo/Library/GD32F30x_Firmware_Library_V2.1.0/`

包含全部外设的 driver C/H：`gd32f30x_{adc,bkp,can,dma,exti,fmc,gpio,i2c,rtc,spi,timer,usart,...}.{c,h}`。每个 example 工程都内嵌一份这个 library 副本。

## CMSIS / 启动文件
路径: `KIT/3. SDK/extracted/GD32F303RCT6 Demo/Examples/<任一>/GD32/CMSIS/`

包含 `core_cm4.h` + `system_gd32f30x.{c,h}` + GD 启动汇编（在每个 example 的 `MDK-ARM/GD/` 下，`.s` 文件，需要从 Keil 工程移植到 GCC 链接脚本）。

## 实践笔记

（执行任务时遇到的 quirk / 反直觉点写在这里）

- _空_
