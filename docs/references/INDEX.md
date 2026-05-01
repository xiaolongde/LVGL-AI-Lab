---
type: references-index
project: LVGL-AI-Lab
updated: 2026-05-01
---

# 项目资料库 — INDEX

> 本目录是 GD32F303RCT6 KIT 资料的**指针 + 提炼**，不复制原文。需要细节直接打开 KIT 源文件。
>
> KIT 路径：`D:\projects\GD\GD32F303RxT6 KIT\`

## 速查表

| 我想知道 | 看哪里 |
|---|---|
| MCU 关键参数 / 时钟树 / RAM-Flash 容量 | [`mcu.md`](mcu.md) |
| 板上接了什么 / 引脚映射全表 | [`board.md`](board.md) |
| 三块屏对比 / 选哪个 / 各 IC 关键参数 | [`screens.md`](screens.md) |
| 官方 SDK 怎么用 / Examples 哪些可抄 | [`sdk.md`](sdk.md) |
| Datasheet PDF / Reference Manual PDF | KIT/`4. DATASHEET/` |
| 板原理图 PDF | KIT/`2. HDK/V5.1/Schematic.pdf` |
| 烧录工具 / Keil / DFU | KIT/`1. TOOLS & DOC/` |

## KIT 源路径速查

```
D:\projects\GD\GD32F303RxT6 KIT\
├── 1. TOOLS & DOC/
│   ├── Keil/extracted/      ← GD32F30x_AddOn V2.2.0 已解压
│   ├── Keil/mdk525.exe      ← Keil MDK5.25 安装包（不用，我们走 CMake）
│   ├── GD32DfuDrivers...rar ← USB DFU 驱动（按需）
│   ├── GD32_MCU_HID_IAP_... ← HID IAP 上位机
│   └── USB_Virtual_Com_...  ← USB CDC 虚拟串口驱动
├── 2. HDK/V5.1/
│   ├── Schematic.pdf        ← 板原理图（关键！）
│   ├── Composite Drawing.pdf
│   └── 64PIN 底板 - V5.1.{Pcb,Sch}Lib  ← Altium 库
├── 3. SDK/extracted/GD32F303RCT6 Demo/
│   ├── Library/
│   │   ├── GD32F30x_Firmware_Library_V2.1.0/  ← 标准外设库
│   │   └── GD32F30x_AddOn_V2.1.0/             ← Keil pack 等价物
│   └── Examples/            ← 7 个工程模板
│       ├── LCD 0.96inch BOE/   ← ST7735S, 160x80
│       ├── LCD 1.14inch/       ← ST7789V3, 240x135
│       ├── LCD 1.47inch/       ← ?, 待查
│       ├── RGB LED/            ← 最简，hello-world 起点
│       ├── RTC/                ← LSE + RTC + BKP
│       ├── USART/              ← UART log
│       └── W25Qxx/             ← SPI flash (W25Q64 8MB on board)
└── 4. DATASHEET/
    ├── GD32F303xx_Datasheet_Rev1.8.pdf       ← 概览 + 引脚 + 电气参数 (~2.5MB)
    ├── GD32F30x_User_Manual_CN_Rev2.8.pdf    ← Reference Manual 中文版 (~13MB)
    └── 屏幕技术资料/
        ├── 0.96 inch/{0.96 寸 IPS 规格书.pdf, ST7735S 芯片手册.pdf}
        └── 1.14 inch/{1.14 寸 IPS 规格书.pdf, ST7789V3_SPEC...pdf}
```

## 我们为什么不直接用 Keil 工程

KIT 自带 7 个 MDK-ARM 工程，但本项目目标是 **CMake + OpenOCD + GDB** 闭环调试链。
迁移路径：抄 Keil 工程的 `Components/`、`Core/`、`GD32/` 三个源码目录 → 写 `CMakeLists.txt` 替换 `MDK-ARM/`，
linker script 自己写（参考 firmware lib 里的 `.s` 启动文件）。

## 后续维护

- 资料库本身不会自动更新——AI 在 case 执行中读到新东西，写到对应文件下方"实践笔记"段
- 不重复 datasheet 内容，只放"读完后我知道但不在原文一句话内的事"
