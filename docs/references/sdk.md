---
type: reference
project: LVGL-AI-Lab
updated: 2026-05-01
topic: KIT 自带 SDK + Example 工程
---

# KIT 自带 SDK + Example 工程指引

> 路径: `KIT/3. SDK/extracted/GD32F303RCT6 Demo/`

## 顶层结构
```
GD32F303RCT6 Demo/
├── Examples/                              ← 7 个 Keil 工程模板
│   ├── LCD 0.96inch BOE/
│   ├── LCD 1.14inch/
│   ├── LCD 1.47inch/
│   ├── RGB LED/                           ← 最简、最适合做 hello-world 起点
│   ├── RTC/
│   ├── USART/
│   └── W25Qxx/
└── Library/
    ├── GD32F30x_Firmware_Library_V2.1.0/  ← 标准外设库（每个 example 内嵌一份副本）
    └── GD32F30x_AddOn_V2.1.0/             ← Keil pack 等价物
```

## 每个 Example 的标准目录骨架
```
<Example>/
├── Components/
│   ├── BSP/             ← 板级支持包（gpio.c/h, spi.c/h, systick.c/h, usart.c/h）
│   └── Devices/         ← 设备驱动（仅当 example 涉及 LCD/W25Qxx 时存在）
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gd32f30x_it.h
│   │   └── gd32f30x_libopt.h    ← 选择启用哪些 standard peripheral 模块
│   └── Src/
│       ├── main.c
│       └── gd32f30x_it.c        ← 中断服务例程
├── GD32/
│   ├── CMSIS/
│   │   ├── core_cm4.h, core_cm4_simd.h, core_cmFunc.h, core_cmInstr.h
│   │   └── GD/GD32F30x/Include/system_gd32f30x.h
│   ├── GD32F30x_standard_peripheral/    ← firmware lib (重点)
│   │   ├── Include/gd32f30x_*.h
│   │   └── Source/gd32f30x_*.c
│   └── GD32F30x_usbd_library/           ← USB device 协议栈（v0.1 不用）
└── MDK-ARM/             ← Keil 工程文件（我们走 CMake，本目录只供参考）
    ├── GD/              ← 启动汇编 .s（需移植到 GCC linker script）
    └── Listings/, Objects/
```

## 推荐 起手路径（CMake 化）

**抄 RGB LED example 做 LED bring-up**（最简，无 LCD/SPI/RTC 噪音）：

1. 复制 `Examples/RGB LED/` 整目录到项目源码区
2. 删 `MDK-ARM/`、`Components/Devices/`（如有）
3. 保留 `Components/BSP/`、`Core/`、`GD32/`
4. 转为 CMake 工程：
   - 写 `cmake/arm-none-eabi.toolchain.cmake`（toolchain file，arm-none-eabi-gcc + 标志）
   - 写 `linker/gd32f303rc.ld`（256K Flash @ 0x08000000 + 48K SRAM @ 0x20000000）
   - 写顶层 `CMakeLists.txt`：`add_executable` + 包含 `GD32/`、`Components/BSP/`、`Core/Src/`
   - 用 GD32 standard peripheral 库内的启动汇编 `.s`（在 `Library/GD32F30x_AddOn_V2.1.0/Device/GD/GD32F30x/Source/GCC/` 下，可能要找一下）
5. 编 → 烧录 → 看 RGB LED 闪 → 录视频

## 起步用哪个 Example

| 任务 | 抄哪个 example | 拿什么 |
|---|---|---|
| LED bring-up（hello-world） | `RGB LED/` | main.c + GPIO 配 + SysTick delay |
| USART log（替代 RTT） | `USART/` | usart.c init + printf 重定向 |
| LCD bring-up | `LCD 1.14inch/` | TFTLCD/tftlcd.{c,h} 完整驱动 |
| RTC + LSE | `RTC/` | RTC init + BKP magic 检测 + 时间结构体转换 |
| SPI Flash（v0.2 字库存储） | `W25Qxx/` | W25Q64 driver |

## 抄 vs 自写 — 方法论决策点

KIT 自带 example 是**双刃剑**：
- 抄起来快，但 AI 是"复制粘贴"还是"理解后改写"差异很大——这是 capability report 的核心观察点
- 建议：每个 sub-task 的 pre 里 AI 必须明确写"这一步我打算抄 X 段还是自己从 datasheet 推导"+ 理由
- post 里写"实际抄了哪些行 / 改了哪些 / 为什么"——这是 Playbook "AI 在有现成代码时的偷懒模式" 章节的素材

## 实践笔记

（执行任务时遇到的 quirk / 反直觉点写在这里）

- _空_
